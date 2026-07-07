// review.cpp
#include "Review.h"
#include <QDebug>

// ===== Constructors =====
Review::Review()
    : reviewId(0)
    , userId(0)
    , bookId(0)
    , text("")
    , rating(0)
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime()) {
}

Review::Review(int reviewId, int userId, int bookId, const QString& text, int rating)
    : reviewId(reviewId)
    , userId(userId)
    , bookId(bookId)
    , text(text)
    , rating(rating)
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime()) {

    // Validate rating
    if (!isValidRating()) {
        qWarning() << "Invalid rating:" << rating << "Setting to 0";
        this->rating = 0;
    }
}
//constructor for DataBase
Review::Review(int reviewId, int userId, int bookId, const QString& text, int rating ,QDateTime createdAt ,QDateTime updatedAt)
    : reviewId(reviewId)
    , userId(userId)
    , bookId(bookId)
    , text(text)
    , rating(rating)
    , createdAt(createdAt)
    , updatedAt(updatedAt){

}
// ===== Setters =====the science lab
void Review::setRating(int rating) {
    if (rating >= 1 && rating <= 5) {
        this->rating = rating;
        updatedAt = QDateTime::currentDateTime();
    } else {
        qWarning() << "Invalid rating:" << rating << "Must be between 1 and 5";
    }
}

// ===== Helper Methods =====
QString Review::getRatingStars() const {
    QString stars;
    for (int i = 0; i < rating; ++i) {
        stars += "★";
    }
    for (int i = rating; i < 5; ++i) {
        stars += "☆";
    }
    return stars;
}