// reviewservice.cpp
#include "ReviewService.h"
#include <QDebug>

// ===== Constructor =====
ReviewService::ReviewService(ReviewRepository* reviewRepo,
                             BookRepository* bookRepo,
                             NotificationService* notifService ,   QObject* parent)
    : reviewRepo(reviewRepo)
    , bookRepo(bookRepo)
    , notifService(notifService) , QObject(parent) {
}

// ===== Review Management =====

bool ReviewService::addReview(int userId, int bookId, const QString& text, int rating) {
    // 1. Validate rating
    if (rating < 1 || rating > 5) {
        qWarning() << "Invalid rating:" << rating << ". Must be between 1 and 5.";
        return false;
    }

    // 2. Check if user already reviewed this book
    if (hasUserReviewed(userId, bookId)) {
        qWarning() << "User" << userId << "has already reviewed book" << bookId;
        return false;
    }

    // 3. Check if book exists
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // 4. Validate review text
    if (text.trimmed().isEmpty()) {
        qWarning() << "Review text cannot be empty";
        return false;
    }

    // 5. Create review
    int newId = reviewRepo->getNextId();
    Review* review = new Review(newId, userId, bookId, text, rating);

    // 6. Add to repository
    if (!reviewRepo->addReview(review)) {
        delete review;
        qWarning() << "Failed to add review to repository";
        return false;
    }

    // 7. Update book's average rating
    updateBookAverageRating(bookId);

    // 8. Send notification
    sendReviewNotification(bookId, rating);

    qDebug() << "Review added for book:" << book->getTitle()
             << "by user:" << userId << "Rating:" << rating;

    return true;
}


bool ReviewService::editReview(int reviewId, int userId, const QString& newText, int newRating) {
    // 1. Validate rating
    if (newRating < 1 || newRating > 5) {
        qWarning() << "Invalid rating:" << newRating << ". Must be between 1 and 5.";
        return false;
    }

    // 2. Validate review text
    if (newText.trimmed().isEmpty()) {
        qWarning() << "Review text cannot be empty";
        return false;
    }

    // 3. Find review
    Review* review = reviewRepo->findById(reviewId);
    if (!review) {
        qWarning() << "Review not found with ID:" << reviewId;
        return false;
    }

    // 4. Check ownership
    if (review->getUserId() != userId) {
        qWarning() << "User" << userId << "does not own review" << reviewId;
        return false;
    }

    // 5. Update review
    int bookId = review->getBookId();
    review->setText(newText);
    review->setRating(newRating);

    // 6. Update in repository
    if (!reviewRepo->updateReview(review)) {
        qWarning() << "Failed to update review";
        return false;
    }

    // 7. Update book's average rating
    updateBookAverageRating(bookId);

    qDebug() << "Review updated:" << reviewId << "New rating:" << newRating;
    return true;
}

bool ReviewService::deleteReview(int reviewId, int userId) {
    // 1. Find review
    Review* review = reviewRepo->findById(reviewId);
    if (!review) {
        qWarning() << "Review not found with ID:" << reviewId;
        return false;
    }

    // 2. Check ownership
    if (review->getUserId() != userId) {
        qWarning() << "User" << userId << "does not own review" << reviewId;
        return false;
    }

    // 3. Get book ID before deletion
    int bookId = review->getBookId();

    // 4. Delete review
    if (!reviewRepo->deleteReview(reviewId)) {
        qWarning() << "Failed to delete review";
        return false;
    }

    // 5. Update book's average rating
    updateBookAverageRating(bookId);

    qDebug() << "Review deleted:" << reviewId << "by user:" << userId;
    return true;
}

bool ReviewService::deleteReviewByAdmin(int reviewId, const QString& reason) {
    Review* review = reviewRepo->findById(reviewId);
    if (!review) {
        qWarning() << "Review not found with ID:" << reviewId;
        return false;
    }

    int bookId = review->getBookId();

    if (!reviewRepo->deleteReview(reviewId)) {
        qWarning() << "Failed to delete review by admin";
        return false;
    }

    updateBookAverageRating(bookId);

    qDebug() << "Review deleted by admin:" << reviewId << "Reason:" << reason;
    return true;
}

// ===== Rating Calculations =====

double ReviewService::getAverageRating(int bookId) const {
    QVector<Review*> reviews = reviewRepo->getReviewsByBookId(bookId);

    if (reviews.isEmpty()) {
        return 0.0;
    }

    int total = 0;
    for (Review* review : reviews) {
        total += review->getRating();
    }

    return static_cast<double>(total) / reviews.size();
}

QMap<int, int> ReviewService::getRatingDistribution(int bookId) const {
    QMap<int, int> distribution;

    // Initialize with zeros
    for (int i = 1; i <= 5; ++i) {
        distribution[i] = 0;
    }

    QVector<Review*> reviews = reviewRepo->getReviewsByBookId(bookId);
    for (Review* review : reviews) {
        int rating = review->getRating();
        distribution[rating] = distribution[rating] + 1;
    }

    return distribution;
}

// ===== Getters =====

QVector<Review*> ReviewService::getReviewsForBook(int bookId) const {
    return reviewRepo->getReviewsByBookId(bookId);
}

QVector<Review*> ReviewService::getReviewsByUser(int userId) const {
    return reviewRepo->getReviewsByUserId(userId);
}

Review* ReviewService::getReviewById(int reviewId) const {
    return reviewRepo->findById(reviewId);
}

bool ReviewService::hasUserReviewed(int userId, int bookId) const {
    return reviewRepo->hasUserReviewed(userId, bookId);
}

Review* ReviewService::getUserReview(int userId, int bookId) const {
    return reviewRepo->getUserReview(userId, bookId);
}

// ===== Private Methods =====

void ReviewService::updateBookAverageRating(int bookId) {
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found for rating update:" << bookId;
        return;
    }

    double newAvg = getAverageRating(bookId);
    book->setAverageRating(newAvg);
    bookRepo->updateBook(book);

    qDebug() << "Updated average rating for book:" << book->getTitle()
             << "->" << newAvg;
}

void ReviewService::sendReviewNotification(int bookId, int rating) {
    if (!notifService) return;

    Book* book = bookRepo->findById(bookId);
    if (!book) return;

    // Send notification to publisher
    QString title = "⭐ New Review!";
    QString message = QString("User gave %1 stars to your book '%2'")
                          .arg(rating)
                          .arg(book->getTitle());

    notifService->sendToRole("Publisher", NotificationType::NewReview, title, message);

    // Also send to users who have saved this book (optional)
    // This would require a "getUsersWhoSavedBook" method
}

QVector<Review*> ReviewService::getAllReviews() const
{

    return reviewRepo->getAllReviews();
}