// purchaserepository.cpp
#include "PurchaseRepository.h"
#include <QDebug>

PurchaseRepository::PurchaseRepository(QObject* parent) : QObject (parent) {
    loadAllFromDatabase();
}

PurchaseRepository::~PurchaseRepository() {
    clearCache();
}

bool PurchaseRepository::addPurchase(Purchase* purchase) {
    if (!purchase) return false;

    int id = purchase->getPurchaseId();
    if (purchasesById.contains(id)) {
        qWarning() << "Purchase with ID" << id << "already exists!";
        return false;
    }

    // 1. Add to cache
    addToCache(purchase);

    // 2. Save to SQLite
    if (!saveToDatabase(purchase)) {
        removeFromCache(id);
        return false;
    }

    // 3. Save purchase items
    if (!savePurchaseItems(id, purchase->getItems())) {
        qWarning() << "Failed to save purchase items!";
        // TODO: Rollback purchase
        return false;
    }

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


    if (!saveToDatabase(purchase)) {
        loadAllFromDatabase();
        return false;
    }
    return true;
}

bool PurchaseRepository::deletePurchase(int purchaseId) {
    Purchase* purchase = purchasesById.value(purchaseId, nullptr);
    if (!purchase) return false;



    // 1. Delete from SQLite first
    if (!deleteFromDatabase(purchaseId)) {
        qWarning() << "Failed to delete purchase from database!";
        return false;
    }

    // 2. Remove from cache
    removeFromCache(purchaseId);
    delete purchase;


    qDebug() << "Purchase deleted:" << purchaseId;
    return true;
}


bool PurchaseRepository::loadAllFromDatabase() {
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Load purchases
    QString query = R"(
        SELECT id, user_id, total_price, discount_amount, final_price,
               status, purchased_at
        FROM purchase
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load purchases:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        Purchase* purchase = new Purchase(
            sqlQuery.value("id").toInt(),
            sqlQuery.value("user_id").toInt(),
            QVector<CartItem>(),  // Items will be loaded separately
            sqlQuery.value("total_price").toDouble(),
            sqlQuery.value("discount_amount").toDouble(),
            sqlQuery.value("final_price").toDouble(),
            QDateTime::fromString(sqlQuery.value("purchased_at").toString(), Qt::ISODate),
            stringToStatus(sqlQuery.value("status").toString())
            );


        // Load purchase items
        loadPurchaseItems(purchase);

        addToCache(purchase);
        count++;
    }

    qDebug() << "✅ Loaded" << count << "purchases from SQLite";
    return true;
}

bool PurchaseRepository::saveToDatabase(Purchase* purchase) {
    if (!purchase) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO purchase (
            id, user_id, total_price, discount_amount, final_price,
            status, purchased_at
        ) VALUES (
            :id, :user_id, :total_price, :discount_amount, :final_price,
            :status, :purchased_at
        )
    )";

    QVariantMap params;
    params["id"] = purchase->getPurchaseId();
    params["user_id"] = purchase->getUserId();
    params["total_price"] = purchase->getTotalPrice();
    params["discount_amount"] = purchase->getDiscountAmount();
    params["final_price"] = purchase->getFinalPrice();
    params["status"] = statusToString(purchase->getStatus());
    params["purchased_at"] = purchase->getPurchasedAt().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}

bool PurchaseRepository::deleteFromDatabase(int purchaseId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // 1. Delete purchase items first (foreign key)
    QString deleteItemsQuery = "DELETE FROM purchase_item WHERE purchase_id = :purchase_id";
    QVariantMap params;
    params["purchase_id"] = purchaseId;
    db->executeQuery(deleteItemsQuery, params);

    // 2. Delete purchase
    QString deletePurchaseQuery = "DELETE FROM purchase WHERE id = :id";
    params.clear();
    params["id"] = purchaseId;
    return db->executeQuery(deletePurchaseQuery, params);
}

bool PurchaseRepository::savePurchaseItems(int purchaseId, const QVector<CartItem>& items) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Delete existing items first
    QString deleteQuery = "DELETE FROM purchase_item WHERE purchase_id = :purchase_id";
    QVariantMap deleteParams;
    deleteParams["purchase_id"] = purchaseId;
    db->executeQuery(deleteQuery, deleteParams);

    // Insert new items
    QString insertQuery = R"(
        INSERT INTO purchase_item (
            purchase_id, book_id, quantity, unit_price, discounted_price
        ) VALUES (
            :purchase_id, :book_id, :quantity, :unit_price, :discounted_price
        )
    )";

    for (const CartItem& item : items) {
        QVariantMap params;
        params["purchase_id"] = purchaseId;
        params["book_id"] = item.getBookId();
        params["quantity"] = item.getQuantity();
        params["unit_price"] = item.getUnitPrice();
        params["discounted_price"] = item.getDiscountedPrice();

        if (!db->executeQuery(insertQuery, params)) {
            return false;
        }
    }

    return true;
}

bool PurchaseRepository::loadPurchaseItems(Purchase* purchase) {
    if (!purchase) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT book_id, quantity, unit_price, discounted_price
        FROM purchase_item
        WHERE purchase_id = :purchase_id
    )";

    QVariantMap params;
    params["purchase_id"] = purchase->getPurchaseId();

    QSqlQuery sqlQuery = db->executeSelect(query, params);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load purchase items:" << sqlQuery.lastError().text();
        return false;
    }

    QVector<CartItem> items;
    while (sqlQuery.next()) {
        CartItem item(
            sqlQuery.value("book_id").toInt(),
            sqlQuery.value("quantity").toInt(),
            sqlQuery.value("unit_price").toDouble(),
            sqlQuery.value("discounted_price").toDouble()
            );
        items.append(item);
    }

    purchase->setItems(items);
    return true;
}

// =============================================
// ===== Helper Methods =====
// =============================================

void PurchaseRepository::addToCache(Purchase* purchase) {
    if (!purchase) return;
    purchasesById[purchase->getPurchaseId()] = purchase;
}

void PurchaseRepository::removeFromCache(int purchaseId) {
    purchasesById.remove(purchaseId);
}

void PurchaseRepository::clearCache() {
    qDeleteAll(purchasesById);
    purchasesById.clear();
}


// ===== Helper Functions =====
PurchaseStatus PurchaseRepository:: stringToStatus(const QString& statusStr)
{
    if (statusStr == "Pending") return PurchaseStatus::Pending;
    if (statusStr == "Completed") return PurchaseStatus::Completed;
    if (statusStr == "Failed") return PurchaseStatus::Failed;
    if (statusStr == "Cancelled") return PurchaseStatus::Cancelled;
    if (statusStr == "Refunded") return PurchaseStatus::Refunded;
    return PurchaseStatus::Pending;
}

QString PurchaseRepository::statusToString(PurchaseStatus status)
{
    switch(status) {
    case PurchaseStatus::Pending: return "Pending";
    case PurchaseStatus::Completed: return "Completed";
    case PurchaseStatus::Failed: return "Failed";
    case PurchaseStatus::Cancelled: return "Cancelled";
    case PurchaseStatus::Refunded: return "Refunded";
    default: return "Pending";
    }
}