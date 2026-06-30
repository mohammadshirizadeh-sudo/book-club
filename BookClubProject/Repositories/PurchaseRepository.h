// purchaserepository.h
#ifndef PURCHASEREPOSITORY_H
#define PURCHASEREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Purchase.h"

/**
 * @brief Repository for managing Purchase objects in memory
 * Handles CRUD operations for purchases
 */
class PurchaseRepository {
private:
    QMap<int, Purchase*> purchasesById;  // Fast lookup by purchase ID

public:
    PurchaseRepository();
    ~PurchaseRepository();

    // ===== CRUD Operations =====


    bool addPurchase(Purchase* purchase);

    Purchase* findById(int id) const;

    QVector<Purchase*> getAllPurchases() const;


    QVector<Purchase*> getPurchasesByUserId(int userId) const;

    QVector<Purchase*> getPurchasesByBookId(int bookId) const;

    bool updatePurchase(Purchase* purchase);

    bool deletePurchase(int purchaseId);
};

#endif // PURCHASEREPOSITORY_H