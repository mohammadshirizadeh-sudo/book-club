// cartservice.h
#ifndef CARTSERVICE_H
#define CARTSERVICE_H

#include <QVector>
#include "../Shared/Cart.h"
#include "../Shared/CartItem.h"
#include "../Repositories/BookRepository.h"

class CartService {
private:
    Cart* cart;                 // Current user's cart
    BookRepository* bookRepo;   // To get book prices and discounts

public:
    // ===== Constructor =====
    explicit CartService(BookRepository* repo);
    ~CartService();

    // ===== Cart Management =====

    void createCart(int userId);

    bool addToCart(int bookId, int quantity = 1);

    bool removeFromCart(int bookId);


    bool updateQuantity(int bookId, int quantity);

    void clearCart();

    // ===== Calculations =====
    void calculateTotal();
    double getTotalPrice() const;
    double getTotalDiscount() const;

    double getFinalPrice() const;

    int getTotalItemCount() const;

    int getUniqueBookCount() const;

    // ===== Getters =====


    QVector<CartItem> getCartItems() const;
    bool isEmpty() const;
    bool contains(int bookId) const;
    CartItem* getCartItem(int bookId);
    const CartItem* getCartItem(int bookId) const;
    Cart* getCart() const { return cart; }
    int getUserId() const;

private:
    double getBookDiscountedPrice(int bookId) const;
};

#endif // CARTSERVICE_H