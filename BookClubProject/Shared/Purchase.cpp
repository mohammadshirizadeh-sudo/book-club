// purchase.cpp
#include "Purchase.h"
#include <QDebug>



Purchase::Purchase()
    : purchaseId(0)
    , userId(0)
    , totalPrice(0.0)
    , discountAmount(0.0)
    , finalPrice(0.0)
    , purchasedAt(QDateTime::currentDateTime())
    , status(PurchaseStatus::Pending) {
}

Purchase::Purchase(int purchaseId, int userId, const QVector<CartItem>& items,
                   double totalPrice, double discountAmount, double finalPrice)
    : purchaseId(purchaseId)
    , userId(userId)
    , items(items)
    , totalPrice(totalPrice)
    , discountAmount(discountAmount)
    , finalPrice(finalPrice)
    , purchasedAt(QDateTime::currentDateTime())
    , status(PurchaseStatus::Pending) {
}

//Helper Methods

QString Purchase::getStatusString() const {
    switch(status) {
    case PurchaseStatus::Pending:   return "Pending";
    case PurchaseStatus::Completed: return "Completed";
    case PurchaseStatus::Failed:    return "Failed";
    case PurchaseStatus::Cancelled: return "Cancelled";
    case PurchaseStatus::Refunded:  return "Refunded";
    default: return "Unknown";
    }
}

int Purchase::getTotalItemCount() const {
    int total = 0;
    for (const CartItem& item : items) {
        total += item.getQuantity();
    }
    return total;
}

QString Purchase::getSummary() const {
    QString summary = QString("Purchase #%1\n")
    .arg(purchaseId);
    summary += QString("User: %1\n").arg(userId);
    summary += QString("Date: %1\n").arg(purchasedAt.toString("yyyy-MM-dd hh:mm"));
    summary += QString("Total Items: %1\n").arg(getTotalItemCount());
    summary += QString("Total Price: $%1\n").arg(totalPrice, 0, 'f', 2);
    summary += QString("Discount: $%1\n").arg(discountAmount, 0, 'f', 2);
    summary += QString("Final Price: $%1\n").arg(finalPrice, 0, 'f', 2);
    summary += QString("Status: %1").arg(getStatusString());
    return summary;
}