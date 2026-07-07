// cartservice.h
#ifndef CARTSERVICE_H
#define CARTSERVICE_H

#include <QVector>
#include <QMap>
#include "../Shared/Cart.h"
#include "../Shared/CartItem.h"
#include "../Repositories/BookRepository.h"
#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"



class CartService: public QObject {
    Q_OBJECT
private:

    QMap<int , Cart*> carts;
    mutable QMutex m_mutex;

    BookRepository* bookRepo;   // To get book prices and discounts
    int currentUserId;

public:
    // ===== Constructor =====
    explicit CartService(BookRepository* repo , QObject* parent = nullptr);
    ~CartService();

    // ===== Cart Management =====

    Cart* getOrcreateCart(int userId);

    bool addToCart(int userId ,int bookId, int quantity = 1);

    bool removeFromCart(int userId, int bookId);



    bool updateQuantity(int userId,int bookId, int quantity);

    void clearCart(int userId);

    // ===== Calculations =====
    void calculateTotal(int userId);
    double getTotalPrice(int userId) const;
    double getTotalDiscount(int userId) const;

    double getFinalPrice(int userId) const;

    int getTotalItemCount(int userId) const;

    int getUniqueBookCount(int userId) const;

    // ===== Getters =====


    QVector<CartItem> getCartItems(int userId) const;
    bool isEmpty(int userId) const;
    bool contains(int userId,int bookId) const;
    CartItem* getCartItem(int userId,int bookId);
    const CartItem* getCartItem(int userId,int bookId) const;
    Cart* getCart(int userId) const ;
    int getUserId(int userId) const;



    bool loadAllFromDatabase();
    bool saveCartToDatabase(Cart* cart);
    bool saveCartItemsToDatabase(int userId, const QVector<CartItem>& items);
    bool deleteCartFromDatabase(int userId);
    bool loadCartItems(Cart* cart);

private:
    double getBookDiscountedPrice(int bookId) const;


    void addToCache(Cart* cart);
    void removeFromCache(int userId);
    void clearCache();




};

#endif // CARTSERVICE_H