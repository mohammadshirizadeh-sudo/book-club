// reviewrepository.cpp
#include "ReviewRepository.h"
#include <QDebug>

ReviewRepository::ReviewRepository() {
    // TODO: Load from file in Phase 3
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

    reviewsById[id] = review;
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
    qDebug() << "Review updated:" << id;
    return true;
}

bool ReviewRepository::deleteReview(int reviewId) {
    Review* review = reviewsById.value(reviewId, nullptr);
    if (!review) {
        qWarning() << "Review with ID" << reviewId << "not found!";
        return false;
    }

    reviewsById.remove(reviewId);
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