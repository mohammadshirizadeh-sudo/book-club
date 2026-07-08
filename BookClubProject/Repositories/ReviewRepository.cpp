// reviewrepository.cpp
#include "ReviewRepository.h"
#include <QDebug>

ReviewRepository::ReviewRepository(QObject* parent) :QObject(parent) {


    loadAllFromDatabase();
}

ReviewRepository::~ReviewRepository() {

}

bool ReviewRepository::addReview(QSharedPointer<Review> review) {

    if (!review) {
        qWarning() << "Review is null!";
        return false;
    }

    int id = review->getReviewId();

    {
         QMutexLocker locker(&m_mutex);
        // Check if review already exists
        if (reviewsById.contains(id)) {
            qWarning() << "Review with ID" << id << "already exists!";
            return false;
        }
        addToCache(review);
    }
    if (!saveToDatabase(review)) {
        QMutexLocker locker(&m_mutex);
        removeFromCache(id);
        return false;
    }

    qDebug() << "Review added:" << id << "for book:" << review->getBookId();
    return true;
}

QSharedPointer<Review> ReviewRepository::findById(int id) const {
    QMutexLocker locker(&m_mutex);
    return reviewsById.value(id, nullptr);
}

QVector<QSharedPointer<Review>> ReviewRepository::getAllReviews() const {
    QMutexLocker locker(&m_mutex);
    return reviewsById.values().toVector();
}

QVector<QSharedPointer<Review>> ReviewRepository::getReviewsByBookId(int bookId) const {

    QMutexLocker locker(&m_mutex);
    QVector<QSharedPointer<Review>> result;

    for (QSharedPointer<Review> review : reviewsById) {
        if (review->getBookId() == bookId) {
            result.append(review);
        }
    }

    return result;
}

QVector<QSharedPointer<Review>> ReviewRepository::getReviewsByUserId(int userId) const {
    QMutexLocker locker(&m_mutex);
    QVector<QSharedPointer<Review>> result;

    for (QSharedPointer<Review> review : reviewsById) {
        if (review->getUserId() == userId) {
            result.append(review);
        }
    }

    return result;
}

bool ReviewRepository::updateReview(QSharedPointer<Review> review) {
    if (!review) {
        qWarning() << "Review is null!";
        return false;
    }


    int id = review->getReviewId();
     QSharedPointer<Review> oldReview = nullptr;
    {
        QMutexLocker locker(&m_mutex);
        if (!reviewsById.contains(id)) {
            qWarning() << "Review with ID" << id << "not found!";
            return false;
        }

        oldReview = reviewsById[id];

        reviewsById[id] = review;

    }



    if (!saveToDatabase(review)) {
        // Rollback: reload from DB
        QMutexLocker locker(&m_mutex);
        reviewsById[id] = oldReview;
        return false;
    }

    qDebug() << "Review updated:" << id;
    return true;
}

bool ReviewRepository::deleteReview(int reviewId) {
    QMutexLocker locker(&m_mutex);
    QSharedPointer<Review> review = reviewsById.value(reviewId, nullptr);
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

    qDebug() << "Review deleted:" << reviewId;
    return true;
}




bool ReviewRepository::hasUserReviewed(int userId, int bookId) const {
    QMutexLocker locker(&m_mutex);
    for (QSharedPointer<Review> review : reviewsById) {
        if (review->getUserId() == userId && review->getBookId() == bookId) {
            return true;
        }
    }
    return false;
}

QSharedPointer<Review> ReviewRepository::getUserReview(int userId, int bookId) const {
    QMutexLocker locker(&m_mutex);
    for (QSharedPointer<Review> review : reviewsById) {
        if (review->getUserId() == userId && review->getBookId() == bookId) {
            return review;
        }
    }
    return nullptr;
}






bool ReviewRepository::loadAllFromDatabase() {
    QMutexLocker locker(&m_mutex);
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
        QSharedPointer<Review> review = QSharedPointer<Review>::create(sqlQuery.value("id").toInt(),
            sqlQuery.value("user_id").toInt(),
            sqlQuery.value("book_id").toInt(),
            sqlQuery.value("text").toString(),
            sqlQuery.value("rating").toInt(),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate),
            QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate));



        addToCache(review);
        count++;
    }

    qDebug() << "✅ Loaded" << count << "reviews from SQLite";
    return true;
}





bool ReviewRepository::saveToDatabase(QSharedPointer<Review> review) {
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


void ReviewRepository::addToCache(QSharedPointer<Review> review) {
    if (!review) return;
    reviewsById[review->getReviewId()] = review;
}

void ReviewRepository::removeFromCache(int reviewId) {

    reviewsById.remove(reviewId);
}

void ReviewRepository::clearCache() {
    reviewsById.clear();
}
