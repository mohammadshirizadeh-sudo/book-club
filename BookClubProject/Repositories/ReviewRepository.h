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
    // QMap<int, QSharedPointer<Review>> reviewsById;
    QMap<int , QSharedPointer<Review>> reviewsById;
    int nextId = 1000;

    mutable QMutex m_mutex;


    void addToCache(QSharedPointer<Review> review);
    void removeFromCache(int reviewId);
    void clearCache();

public:
    ReviewRepository(QObject* parent = nullptr);
    ~ReviewRepository();

    // ===== CRUD Operations =====


    bool addReview(QSharedPointer<Review> review);

    QSharedPointer<Review> findById(int id) const;

    QVector<QSharedPointer<Review>> getAllReviews() const;

    QVector<QSharedPointer<Review>> getReviewsByBookId(int bookId) const;

    QVector<QSharedPointer<Review>> getReviewsByUserId(int userId) const;
    bool updateReview(QSharedPointer<Review> review);
    bool deleteReview(int reviewId);
    bool hasUserReviewed(int userId, int bookId) const;

    QSharedPointer<Review> getUserReview(int userId, int bookId) const;

    int getNextId() { return nextId++; }


    // ===== SQLite Operations (Persistence) =====
    bool loadAllFromDatabase();
    bool saveToDatabase(QSharedPointer<Review> review);
    bool deleteFromDatabase(int reviewId);

};

#endif // REVIEWREPOSITORY_H