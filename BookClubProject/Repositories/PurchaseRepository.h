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
    QMap<int, Purchase*> purchasesById;  // Fast lookup by purchase ID


    void addToCache(Purchase* purchase);
    void removeFromCache(int purchaseId);
    void clearCache();

public:
    PurchaseRepository(QObject* parent= nullptr);
    ~PurchaseRepository();

    // ===== CRUD Operations =====


    bool addPurchase(Purchase* purchase);

    Purchase* findById(int id) const;

    QVector<Purchase*> getAllPurchases() const;


    QVector<Purchase*> getPurchasesByUserId(int userId) const;

    QVector<Purchase*> getPurchasesByBookId(int bookId) const;

    bool updatePurchase(Purchase* purchase);

    bool deletePurchase(int purchaseId);

    bool loadAllFromDatabase();
    bool saveToDatabase(Purchase* purchase);
    bool deleteFromDatabase(int purchaseId);
    bool savePurchaseItems(int purchaseId, const QVector<CartItem>& items);
    bool loadPurchaseItems(Purchase* purchase);
    static PurchaseStatus stringToStatus(const QString& statusStr);

    static QString statusToString(PurchaseStatus status);
};

#endif // PURCHASEREPOSITORY_H