// reviewservice.h
#ifndef REVIEWSERVICE_H
#define REVIEWSERVICE_H

#include <QString>
#include <QVector>
#include "../Shared/Review.h"
#include "../Repositories/ReviewRepository.h"
#include "../Repositories/BookRepository.h"
#include "NotificationService.h"


/**
 * @brief Review Service - Handles review and rating operations
 */
class ReviewService {
private:
    ReviewRepository* reviewRepo;
    BookRepository* bookRepo;
    NotificationService* notifService;

public:
    // ===== Constructor =====
    ReviewService(ReviewRepository* reviewRepo,
                  BookRepository* bookRepo,
                  NotificationService* notifService);

    // ===== Review Management =====

    /**
     * @brief Add a new review for a book
     * @param userId User ID
     * @param bookId Book ID
     * @param text Review text
     * @param rating Rating (1-5)
     * @return true if added successfully
     */
    bool addReview(int userId, int bookId, const QString& text, int rating);

    /**
     * @brief Edit an existing review
     * @param reviewId Review ID
     * @param userId User ID (for ownership check)
     * @param newText New review text
     * @param newRating New rating (1-5)
     * @return true if edited successfully
     */
    bool editReview(int reviewId, int userId, const QString& newText, int newRating);

    /**
     * @brief Delete a review
     * @param reviewId Review ID
     * @param userId User ID (for ownership check)
     * @return true if deleted successfully
     */
    bool deleteReview(int reviewId, int userId);

    /**
     * @brief Delete a review (admin override)
     * @param reviewId Review ID
     * @param reason Reason for deletion
     * @return true if deleted successfully
     */
    bool deleteReviewByAdmin(int reviewId, const QString& reason = "");

    // ===== Rating Calculations =====

    /**
     * @brief Get average rating for a book
     * @param bookId Book ID
     * @return Average rating (0.0 if no reviews)
     */
    double getAverageRating(int bookId) const;

    /**
     * @brief Get rating distribution for a book
     * @param bookId Book ID
     * @return Map of rating -> count (e.g., {5: 10, 4: 5, 3: 2, 2: 1, 1: 0})
     */
    QMap<int, int> getRatingDistribution(int bookId) const;

    // ===== Getters =====

    /**
     * @brief Get all reviews for a book
     * @param bookId Book ID
     * @return Vector of reviews
     */
    QVector<Review*> getReviewsForBook(int bookId) const;

    /**
     * @brief Get all reviews by a user
     * @param userId User ID
     * @return Vector of reviews
     */
    QVector<Review*> getReviewsByUser(int userId) const;

    /**
     * @brief Get a review by ID
     * @param reviewId Review ID
     * @return Review pointer or nullptr
     */
    Review* getReviewById(int reviewId) const;

    /**
     * @brief Check if user has reviewed a book
     * @param userId User ID
     * @param bookId Book ID
     * @return true if user has reviewed this book
     */
    bool hasUserReviewed(int userId, int bookId) const;

    /**
     * @brief Get user's review for a book
     * @param userId User ID
     * @param bookId Book ID
     * @return Review pointer or nullptr
     */
    Review* getUserReview(int userId, int bookId) const;

private:
    /**
     * @brief Update book's average rating
     * @param bookId Book ID
     */
    void updateBookAverageRating(int bookId);

    /**
     * @brief Send notification for new review
     * @param bookId Book ID
     * @param rating Rating given
     */
    void sendReviewNotification(int bookId, int rating);
};

#endif // REVIEWSERVICE_H