// purchaseservice.h
#ifndef PURCHASESERVICE_H
#define PURCHASESERVICE_H

#include <QVector>
#include "../Shared/Purchase.h"
#include "../Shared/Cart.h"
#include "../Repositories/PurchaseRepository.h"
#include "../Repositories/BookRepository.h"
#include "../Repositories/LibraryRepository.h"
#include "CartService.h"
#include "NotificationService.h"


class PurchaseService : public QObject{
    Q_OBJECT
private:
    PurchaseRepository* purchaseRepo;
    BookRepository* bookRepo;
    LibraryRepository* libraryRepo;
    CartService* cartService;
    NotificationService* notifService;
    int nextPurchaseId = 5000;

public:
    // ===== Constructor =====
    PurchaseService(PurchaseRepository* purchaseRepo,
                    BookRepository* bookRepo,
                    LibraryRepository* libraryRepo,
                    CartService* cartService,
                    NotificationService* notifService , QObject* parent = nullptr);

    // ===== Purchase Operations =====


    QSharedPointer<Purchase> checkout(int userId);

    QSharedPointer<Purchase> createPurchase(int userId, const QVector<CartItem>& cartItems,
                             double totalPrice, double discountAmount, double finalPrice);


    bool moveToLibrary(int userId, const QVector<CartItem>& purchaseItems);

    // ===== Purchase History =====

    QVector<QSharedPointer<Purchase>> getPurchaseHistory(int userId) const;
    QSharedPointer<Purchase> getPurchaseById(int purchaseId) const;
    QVector<QSharedPointer<Purchase>> getAllPurchases() const;

    QVector<QSharedPointer<Purchase>> getPurchasesForBook(int bookId) const;

    // ===== Status Management =====
    bool updatePurchaseStatus(int purchaseId, PurchaseStatus newStatus);
    bool cancelPurchase(int purchaseId);
    bool refundPurchase(int purchaseId);


    int getPurchaseCount(int userId) const;

private:

    bool processPayment(double amount);
    void updateBookSales(const QVector<CartItem>& purchaseItems);

    void sendPurchaseConfirmation(int userId, QSharedPointer<Purchase> purchase);
};

#endif // PURCHASESERVICE_H