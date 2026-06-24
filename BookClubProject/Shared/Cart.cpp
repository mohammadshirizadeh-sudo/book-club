// cart.cpp
#include "Cart.h"
#include <QDebug>

// ===== Constructors =====
Cart::Cart()
    : userId(0)
    , totalPrice(0.0)
    , totalDiscount(0.0)
    , finalPrice(0.0) {

}

Cart::Cart(int userId)
    : userId(userId)
    , totalPrice(0.0)
    , totalDiscount(0.0)
    , finalPrice(0.0) {
}

// ===== Getters =====
int Cart::getTotalItems() const {
    int total = 0;
    for (const CartItem& item : items) {
        total += item.getQuantity();
    }
    return total;
}

// ===== Cart Management =====

bool Cart::addItem(const CartItem& item) {
    if (!item.isValid()) {
        qWarning() << "Invalid cart item!";
        return false;
    }

    // Check if item already exists in cart
    for (CartItem& existing : items) {
        if (existing.getBookId() == item.getBookId()) {
            // Update quantity
            int newQuantity = existing.getQuantity() + item.getQuantity();
            existing.setQuantity(newQuantity);
            calculateTotals();
            return true;
        }
    }

    // Add new item
    items.append(item);
    calculateTotals();
    return true;
}

bool Cart::removeItem(int bookId) {
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].getBookId() == bookId) {
            items.remove(i);
            calculateTotals();
            return true;
        }
    }
    return false;
}

bool Cart::updateQuantity(int bookId, int quantity) {
    if (quantity <= 0) {
        return removeItem(bookId);  // If quantity <= 0, remove the item
    }

    for (CartItem& item : items) {
        if (item.getBookId() == bookId) {
            item.setQuantity(quantity);
            calculateTotals();
            return true;
        }
    }
    return false;
}

void Cart::clear() {
    items.clear();
    totalPrice = 0.0;
    totalDiscount = 0.0;
    finalPrice = 0.0;
}

bool Cart::contains(int bookId) const {
    for (const CartItem& item : items) {
        if (item.getBookId() == bookId) {
            return true;
        }
    }
    return false;
}

CartItem* Cart::getItem(int bookId) {
    for (CartItem& item : items) {
        if (item.getBookId() == bookId) {
            return &item;
        }
    }
    return nullptr;
}

const CartItem* Cart::getItem(int bookId) const {
    for (const CartItem& item : items) {
        if (item.getBookId() == bookId) {
            return &item;
        }
    }
    return nullptr;
}

void Cart::calculateTotals() {
    totalPrice = 0.0;
    totalDiscount = 0.0;
    finalPrice = 0.0;

    for (const CartItem& item : items) {
        totalPrice += item.getTotalPrice();
        totalDiscount += item.getDiscountAmount();
        finalPrice += item.getTotalDiscountedPrice();
    }
}