// purchaserepository.cpp
#include "PurchaseRepository.h"
#include <QDebug>

PurchaseRepository::PurchaseRepository() {
}

PurchaseRepository::~PurchaseRepository() {
    qDeleteAll(purchasesById);
}

bool PurchaseRepository::addPurchase(Purchase* purchase) {
    if (!purchase) return false;

    int id = purchase->getPurchaseId();
    if (purchasesById.contains(id)) {
        qWarning() << "Purchase with ID" << id << "already exists!";
        return false;
    }

    purchasesById[id] = purchase;
    return true;
}

Purchase* PurchaseRepository::findById(int id) const {
    return purchasesById.value(id, nullptr);
}

QVector<Purchase*> PurchaseRepository::getAllPurchases() const {
    return purchasesById.values().toVector();
}

QVector<Purchase*> PurchaseRepository::getPurchasesByUserId(int userId) const {
    QVector<Purchase*> result;
    for (Purchase* purchase : purchasesById) {
        if (purchase->getUserId() == userId) {
            result.append(purchase);
        }
    }
    return result;
}

QVector<Purchase*> PurchaseRepository::getPurchasesByBookId(int bookId) const {
    QVector<Purchase*> result;
    for (Purchase* purchase : purchasesById) {
        for (const CartItem& item : purchase->getItems()) {
            if (item.getBookId() == bookId) {
                result.append(purchase);
                break;
            }
        }
    }
    return result;
}

bool PurchaseRepository::updatePurchase(Purchase* purchase) {
    if (!purchase) return false;

    int id = purchase->getPurchaseId();
    if (!purchasesById.contains(id)) {
        qWarning() << "Purchase with ID" << id << "not found!";
        return false;
    }

    purchasesById[id] = purchase;
    return true;
}

bool PurchaseRepository::deletePurchase(int purchaseId) {
    Purchase* purchase = purchasesById.value(purchaseId, nullptr);
    if (!purchase) return false;

    purchasesById.remove(purchaseId);
    delete purchase;
    return true;
}