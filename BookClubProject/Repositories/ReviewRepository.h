// reviewrepository.h
#ifndef REVIEWREPOSITORY_H
#define REVIEWREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Review.h"

/**
 * @brief Repository for managing Review objects in memory
 * Handles CRUD operations for reviews
 */
class ReviewRepository {
private:
    QMap<int, Review*> reviewsById;  // Fast lookup by review ID
    int nextId = 1000;               // Auto-increment ID for new reviews

public:
    ReviewRepository();
    ~ReviewRepository();

    // ===== CRUD Operations =====

    /**
     * @brief Add a new review to repository
     * @param review Review to add
     * @return true if added successfully
     */
    bool addReview(Review* review);

    /**
     * @brief Find review by ID
     * @param id Review ID
     * @return Review pointer or nullptr if not found
     */
    Review* findById(int id) const;

    /**
     * @brief Get all reviews
     * @return Vector of all reviews
     */
    QVector<Review*> getAllReviews() const;

    /**
     * @brief Get reviews by book ID
     * @param bookId Book ID
     * @return Vector of reviews for that book
     */
    QVector<Review*> getReviewsByBookId(int bookId) const;

    /**
     * @brief Get reviews by user ID
     * @param userId User ID
     * @return Vector of reviews by that user
     */
    QVector<Review*> getReviewsByUserId(int userId) const;

    /**
     * @brief Update an existing review
     * @param review Updated review
     * @return true if update was successful
     */
    bool updateReview(Review* review);

    /**
     * @brief Delete a review
     * @param reviewId ID of review to delete
     * @return true if deleted successfully
     */
    bool deleteReview(int reviewId);

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

    /**
     * @brief Get next available ID
     * @return New ID for review
     */
    int getNextId() { return nextId++; }
};

#endif // REVIEWREPOSITORY_H