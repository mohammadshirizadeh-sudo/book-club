// purchaserepository.h
#ifndef PURCHASEREPOSITORY_H
#define PURCHASEREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Purchase.h"
#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"

class PurchaseRepository : public QObject {
    Q_OBJECT
private:
    QMap<int, QSharedPointer<Purchase>> purchasesById;
     mutable QMutex m_mutex;


    void addToCache(QSharedPointer<Purchase> purchase);
    void removeFromCache(int purchaseId);
    void clearCache();

public:
    PurchaseRepository(QObject* parent= nullptr);
    ~PurchaseRepository();

    // ===== CRUD Operations =====


    bool addPurchase(QSharedPointer<Purchase> purchase);

    QSharedPointer<Purchase> findById(int id) const;

    QVector<QSharedPointer<Purchase>> getAllPurchases() const;


    QVector<QSharedPointer<Purchase>> getPurchasesByUserId(int userId) const;

    QVector<QSharedPointer<Purchase>> getPurchasesByBookId(int bookId) const;

    bool updatePurchase(QSharedPointer<Purchase> purchase);

    bool deletePurchase(int purchaseId);

    bool loadAllFromDatabase();
    bool saveToDatabase(QSharedPointer<Purchase> purchase);
    bool deleteFromDatabase(int purchaseId);
    bool savePurchaseItems(int purchaseId, const QVector<CartItem>& items);
    bool loadPurchaseItems(QSharedPointer<Purchase> purchase);
    static PurchaseStatus stringToStatus(const QString& statusStr);

    static QString statusToString(PurchaseStatus status);
    int getMaxPurchaseId() const;
};

#endif // PURCHASEREPOSITORY_H