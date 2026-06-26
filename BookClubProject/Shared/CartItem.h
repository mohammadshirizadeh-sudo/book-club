// cartitem.h
#ifndef CARTITEM_H
#define CARTITEM_H

#include <QString>

class CartItem {
private:
    int bookId;
    int quantity;
    double unitPrice;        // Original price per unit
    double discountedPrice;  // Price per unit after discount

public:
    // ===== Constructors =====
    CartItem();
    CartItem(int bookId, int quantity, double unitPrice, double discountedPrice);

    // ===== Getters =====
    int getBookId() const { return bookId; }
    int getQuantity() const { return quantity; }
    double getUnitPrice() const { return unitPrice; }
    double getDiscountedPrice() const { return discountedPrice; }

    // ===== Setters =====
    void setBookId(int id) { bookId = id; }
    void setQuantity(int qty) {
        if (qty > 0) quantity = qty;
    }
    void setUnitPrice(double price) { unitPrice = price; }
    void setDiscountedPrice(double price) { discountedPrice = price; }

    // ===== Helper Methods =====
    double getTotalPrice() const { return unitPrice * quantity; }
    double getTotalDiscountedPrice() const { return discountedPrice * quantity; }
    double getDiscountAmount() const { return (unitPrice - discountedPrice) * quantity; }
    bool isValid() const { return bookId > 0 && quantity > 0 && unitPrice >= 0; }
};

#endif // CARTITEM_H