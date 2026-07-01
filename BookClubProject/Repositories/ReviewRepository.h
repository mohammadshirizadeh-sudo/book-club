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


    bool addReview(Review* review);

    Review* findById(int id) const;

    QVector<Review*> getAllReviews() const;

    QVector<Review*> getReviewsByBookId(int bookId) const;

    QVector<Review*> getReviewsByUserId(int userId) const;
    bool updateReview(Review* review);
    bool deleteReview(int reviewId);
    bool hasUserReviewed(int userId, int bookId) const;

    Review* getUserReview(int userId, int bookId) const;

    int getNextId() { return nextId++; }
};

#endif // REVIEWREPOSITORY_H