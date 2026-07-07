// reviewrepository.h
#ifndef REVIEWREPOSITORY_H
#define REVIEWREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Review.h"
#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"

class ReviewRepository : public QObject {
    Q_OBJECT
private:
    QMap<int, Review*> reviewsById;
    int nextId = 1000;

    mutable QMutex m_mutex;


    void addToCache(Review* review);
    void removeFromCache(int reviewId);
    void clearCache();

public:
    ReviewRepository(QObject* parent = nullptr);
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


    // ===== SQLite Operations (Persistence) =====
    bool loadAllFromDatabase();
    bool saveToDatabase(Review* review);
    bool deleteFromDatabase(int reviewId);

};

#endif // REVIEWREPOSITORY_H