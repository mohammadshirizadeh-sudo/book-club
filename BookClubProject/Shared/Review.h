// review.h
#ifndef REVIEW_H
#define REVIEW_H

#include <QString>
#include <QDateTime>


class Review {
private:
    int reviewId;
    int userId;
    int bookId;
    QString text;

    int rating;

    QDateTime createdAt;
    QDateTime updatedAt;

    QString status;
    bool isFlagged;

public:
    // ===== Constructors =====
    Review();
    Review(int reviewId, int userId, int bookId, const QString& text, int rating);
    Review(int reviewId, int userId, int bookId, const QString& text, int rating, const QString& status, bool isFlagged ,QDateTime createdAt ,QDateTime updatedAt);

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
    QString getStatus() const;
    void setStatus(const QString &newStatus);
    bool getIsFlagged() const;
    void setIsFlagged(bool newIsFlagged);
};

#endif // REVIEW_H