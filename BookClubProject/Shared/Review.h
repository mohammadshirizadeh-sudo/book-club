// review.h
#ifndef REVIEW_H
#define REVIEW_H

#include <QString>
#include <QDateTime>

/**
 * @brief Review class for user book reviews and ratings
 */
class Review {
private:
    int reviewId;
    int userId;
    int bookId;
    QString text;
    int rating;          // 1 to 5 stars
    QDateTime createdAt;
    QDateTime updatedAt;

public:
    // ===== Constructors =====
    Review();
    Review(int reviewId, int userId, int bookId, const QString& text, int rating);

    // ===== Getters =====
    int getReviewId() const { return reviewId; }
    int getUserId() const { return userId; }
    int getBookId() const { return bookId; }
    QString getText() const { return text; }
    int getRating() const { return rating; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getUpdatedAt() const { return updatedAt; }

    // ===== Setters =====
    void setReviewId(int id) { reviewId = id; }
    void setUserId(int id) { userId = id; }
    void setBookId(int id) { bookId = id; }
    void setText(const QString& text) {
        this->text = text;
        updatedAt = QDateTime::currentDateTime();
    }
    void setRating(int rating);
    void setCreatedAt(const QDateTime& time) { createdAt = time; }
    void setUpdatedAt(const QDateTime& time) { updatedAt = time; }

    // ===== Helper Methods =====
    bool isValidRating() const { return rating >= 1 && rating <= 5; }
    QString getRatingStars() const;
};

#endif // REVIEW_H