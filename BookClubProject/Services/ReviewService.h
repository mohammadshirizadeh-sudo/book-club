// reviewservice.h
#ifndef REVIEWSERVICE_H
#define REVIEWSERVICE_H

#include <QString>
#include <QVector>
#include "../Shared/Review.h"
#include "../Repositories/ReviewRepository.h"
#include "../Repositories/BookRepository.h"
#include "NotificationService.h"

class ReviewService : public QObject{
    Q_OBJECT
private:
    ReviewRepository* reviewRepo;
    BookRepository* bookRepo;
    NotificationService* notifService;

public:
    // ===== Constructor =====
    ReviewService(ReviewRepository* reviewRepo,
                  BookRepository* bookRepo,
                  NotificationService* notifService , QObject* parent = nullptr);

    // ===== Review Management =====

    bool addReview(int userId, int bookId, const QString& text, int rating);

    bool editReview(int reviewId, int userId, const QString& newText, int newRating);
    bool deleteReview(int reviewId, int userId);

    bool deleteReviewByAdmin(int reviewId);

    // ===== Rating Calculations =====

    double getAverageRating(int bookId) const;


    QMap<int, int> getRatingDistribution(int bookId) const;

    // ===== Getters =====

    QVector<QSharedPointer<Review>> getReviewsForBook(int bookId) const;

    QVector<QSharedPointer<Review>> getReviewsByUser(int userId) const;


    QSharedPointer<Review> getReviewById(int reviewId) const;

    bool hasUserReviewed(int userId, int bookId) const;

    QSharedPointer<Review> getUserReview(int userId, int bookId) const;
    QVector<QSharedPointer<Review>> getAllReviews() const;
private:

    void updateBookAverageRating(int bookId);

    void sendReviewNotification(int bookId, int rating);


};

#endif // REVIEWSERVICE_H