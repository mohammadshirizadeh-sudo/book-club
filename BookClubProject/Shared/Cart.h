// cart.h
#ifndef CART_H
#define CART_H

#include <QVector>
#include "CartItem.h"

class Cart {
private:
    int userId;
    QVector<CartItem> items;
    double totalPrice;
    double totalDiscount;
    double finalPrice;

public:
    // ===== Constructors =====
    Cart();
    explicit Cart(int userId);

    // ===== Getters =====
    int getUserId() const { return userId; }
    QVector<CartItem> getItems() const { return items; }
    double getTotalPrice() const { return totalPrice; }
    double getTotalDiscount() const { return totalDiscount; }
    double getFinalPrice() const { return finalPrice; }
    int getItemCount() const { return items.size(); }
    int getTotalItems() const;  // Total quantity of all items

    // ===== Setters =====
    void setUserId(int id) { userId = id; }

    bool addItem(const CartItem& item);
    bool removeItem(int bookId);
    bool updateQuantity(int bookId, int quantity);
    void clear();
    bool contains(int bookId) const;
    CartItem* getItem(int bookId);
    const CartItem* getItem(int bookId) const;
    void calculateTotals();
    bool isEmpty() const { return items.isEmpty(); }
};

#endif // CART_H