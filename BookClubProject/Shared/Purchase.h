// purchase.h
#ifndef PURCHASE_H
#define PURCHASE_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include "CartItem.h"

enum class PurchaseStatus {
    Pending,
    Completed,
    Failed,
    Cancelled,
    Refunded
};

class Purchase {
private:
    int purchaseId;
    int userId;
    QVector<CartItem> items;
    double totalPrice;
    double discountAmount;
    double finalPrice;
    QDateTime purchasedAt;
    PurchaseStatus status;

public:
    // ===== Constructors =====
    Purchase();
    Purchase(int purchaseId, int userId, const QVector<CartItem>& items,
             double totalPrice, double discountAmount, double finalPrice);

    Purchase(int purchaseId, int userId, const QVector<CartItem>& items,
             double totalPrice, double discountAmount, double finalPrice, QDateTime purchasedAt, PurchaseStatus status);


    // ===== Getters =====
    int getPurchaseId() const { return purchaseId; }
    int getUserId() const { return userId; }
    QVector<CartItem> getItems() const { return items; }
    double getTotalPrice() const { return totalPrice; }
    double getDiscountAmount() const { return discountAmount; }
    double getFinalPrice() const { return finalPrice; }
    QDateTime getPurchasedAt() const { return purchasedAt; }
    PurchaseStatus getStatus() const { return status; }

    // ===== Setters =====
    void setPurchaseId(int id) { purchaseId = id; }
    void setUserId(int id) { userId = id; }
    void setItems(const QVector<CartItem>& items) { this->items = items; }
    void setTotalPrice(double price) { totalPrice = price; }
    void setDiscountAmount(double discount) { discountAmount = discount; }
    void setFinalPrice(double price) { finalPrice = price; }
    void setPurchasedAt(const QDateTime& time) { purchasedAt = time; }
    void setStatus(PurchaseStatus status) { this->status = status; }

    // ===== Helper Methods =====

    QString getStatusString() const;

    bool isCompleted() const { return status == PurchaseStatus::Completed; }

    bool isPending() const { return status == PurchaseStatus::Pending; }

    bool isCancelled() const { return status == PurchaseStatus::Cancelled; }

    int getTotalItemCount() const;

    QString getSummary() const;
};

#endif // PURCHASE_H