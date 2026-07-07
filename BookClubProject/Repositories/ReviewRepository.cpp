// reviewrepository.cpp
#include "ReviewRepository.h"
#include <QDebug>

ReviewRepository::ReviewRepository(QObject* parent) :QObject(parent) {


    loadAllFromDatabase();
}

ReviewRepository::~ReviewRepository() {
    // Clean up all reviews to prevent memory leak
    qDeleteAll(reviewsById);
}

bool ReviewRepository::addReview(Review* review) {
    if (!review) {
        qWarning() << "Review is null!";
        return false;
    }

    int id = review->getReviewId();

    // Check if review already exists
    if (reviewsById.contains(id)) {
        qWarning() << "Review with ID" << id << "already exists!";
        return false;
    }



    addToCache(review);

    // 2. Save to SQLite
    if (!saveToDatabase(review)) {
        // Rollback: remove from cache if DB fails
        removeFromCache(id);
        return false;
    }

    qDebug() << "Review added:" << id << "for book:" << review->getBookId();
    return true;
}

Review* ReviewRepository::findById(int id) const {
    return reviewsById.value(id, nullptr);
}

QVector<Review*> ReviewRepository::getAllReviews() const {
    return reviewsById.values().toVector();
}

QVector<Review*> ReviewRepository::getReviewsByBookId(int bookId) const {
    QVector<Review*> result;

    for (Review* review : reviewsById) {
        if (review->getBookId() == bookId) {
            result.append(review);
        }
    }

    return result;
}

QVector<Review*> ReviewRepository::getReviewsByUserId(int userId) const {
    QVector<Review*> result;

    for (Review* review : reviewsById) {
        if (review->getUserId() == userId) {
            result.append(review);
        }
    }

    return result;
}

bool ReviewRepository::updateReview(Review* review) {
    if (!review) {
        qWarning() << "Review is null!";
        return false;
    }

    int id = review->getReviewId();
    if (!reviewsById.contains(id)) {
        qWarning() << "Review with ID" << id << "not found!";
        return false;
    }

    reviewsById[id] = review;


    if (!saveToDatabase(review)) {
        // Rollback: reload from DB
        loadAllFromDatabase();
        return false;
    }

    qDebug() << "Review updated:" << id;
    return true;
}

bool ReviewRepository::deleteReview(int reviewId) {
    Review* review = reviewsById.value(reviewId, nullptr);
    if (!review) {
        qWarning() << "Review with ID" << reviewId << "not found!";
        return false;
    }



    // 1. Delete from SQLite first
    if (!deleteFromDatabase(reviewId)) {
        qWarning() << "Failed to delete review from database!";
        return false;
    }

    // 2. Remove from cache
    removeFromCache(reviewId);
    delete review;

    qDebug() << "Review deleted:" << reviewId;
    return true;
}




bool ReviewRepository::hasUserReviewed(int userId, int bookId) const {
    for (Review* review : reviewsById) {
        if (review->getUserId() == userId && review->getBookId() == bookId) {
            return true;
        }
    }
    return false;
}

Review* ReviewRepository::getUserReview(int userId, int bookId) const {
    for (Review* review : reviewsById) {
        if (review->getUserId() == userId && review->getBookId() == bookId) {
            return review;
        }
    }
    return nullptr;
}






bool ReviewRepository::loadAllFromDatabase() {
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT id, user_id, book_id, text, rating, created_at, updated_at
        FROM review
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load reviews:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        Review* review = new Review(
            sqlQuery.value("id").toInt(),
            sqlQuery.value("user_id").toInt(),
            sqlQuery.value("book_id").toInt(),
            sqlQuery.value("text").toString(),
            sqlQuery.value("rating").toInt(),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate),
            QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate)
            );



        addToCache(review);
        count++;
    }

    qDebug() << "✅ Loaded" << count << "reviews from SQLite";
    return true;
}





bool ReviewRepository::saveToDatabase(Review* review) {
    if (!review) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO review (
            id, user_id, book_id, text, rating, created_at, updated_at
        ) VALUES (
            :id, :user_id, :book_id, :text, :rating, :created_at, :updated_at
        )
    )";

    QVariantMap params;
    params["id"] = review->getReviewId();
    params["user_id"] = review->getUserId();
    params["book_id"] = review->getBookId();
    params["text"] = review->getText();
    params["rating"] = review->getRating();
    params["created_at"] = review->getCreatedAt().toString(Qt::ISODate);
    params["updated_at"] = review->getUpdatedAt().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}


bool ReviewRepository::deleteFromDatabase(int reviewId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = "DELETE FROM review WHERE id = :id";
    QVariantMap params;
    params["id"] = reviewId;

    return db->executeQuery(query, params);
}


void ReviewRepository::addToCache(Review* review) {
    if (!review) return;
    reviewsById[review->getReviewId()] = review;
}

void ReviewRepository::removeFromCache(int reviewId) {
    reviewsById.remove(reviewId);
}

void ReviewRepository::clearCache() {
    qDeleteAll(reviewsById);
    reviewsById.clear();
}
