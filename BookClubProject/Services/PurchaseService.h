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

/**
 * @brief Purchase Service - Handles checkout and purchase operations
 */
class PurchaseService {
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
                    NotificationService* notifService);

    // ===== Purchase Operations =====

    /**
     * @brief Checkout the current cart (create purchase and process payment)
     * @param userId User ID making the purchase
     * @return Purchase pointer if successful, nullptr otherwise
     */
    Purchase* checkout(int userId);

    /**
     * @brief Create a purchase from cart data
     * @param userId User ID
     * @param cartItems Items in cart
     * @param totalPrice Total price before discount
     * @param discountAmount Total discount amount
     * @param finalPrice Final price after discount
     * @return Purchase pointer
     */
    Purchase* createPurchase(int userId, const QVector<CartItem>& cartItems,
                             double totalPrice, double discountAmount, double finalPrice);

    /**
     * @brief Move purchased books to user's library
     * @param userId User ID
     * @param purchaseItems Items purchased
     * @return true if moved successfully
     */
    bool moveToLibrary(int userId, const QVector<CartItem>& purchaseItems);

    // ===== Purchase History =====

    /**
     * @brief Get purchase history for a user
     * @param userId User ID
     * @return Vector of purchases
     */
    QVector<Purchase*> getPurchaseHistory(int userId) const;

    /**
     * @brief Get purchase by ID
     * @param purchaseId Purchase ID
     * @return Purchase pointer or nullptr
     */
    Purchase* getPurchaseById(int purchaseId) const;

    /**
     * @brief Get all purchases (for admin)
     * @return Vector of all purchases
     */
    QVector<Purchase*> getAllPurchases() const;

    /**
     * @brief Get purchases for a book
     * @param bookId Book ID
     * @return Vector of purchases containing this book
     */
    QVector<Purchase*> getPurchasesForBook(int bookId) const;

    // ===== Status Management =====

    /**
     * @brief Update purchase status
     * @param purchaseId Purchase ID
     * @param newStatus New status
     * @return true if updated successfully
     */
    bool updatePurchaseStatus(int purchaseId, PurchaseStatus newStatus);

    /**
     * @brief Cancel a purchase
     * @param purchaseId Purchase ID
     * @return true if cancelled successfully
     */
    bool cancelPurchase(int purchaseId);

    /**
     * @brief Refund a purchase
     * @param purchaseId Purchase ID
     * @return true if refunded successfully
     */
    bool refundPurchase(int purchaseId);

private:
    /**
     * @brief Process payment (simulated)
     * @param amount Amount to charge
     * @return true if payment successful
     */
    bool processPayment(double amount);

    /**
     * @brief Update book sales count after purchase
     * @param purchaseItems Items purchased
     */
    void updateBookSales(const QVector<CartItem>& purchaseItems);

    /**
     * @brief Send purchase confirmation notification
     * @param userId User ID
     * @param purchase Purchase object
     */
    void sendPurchaseConfirmation(int userId, Purchase* purchase);
};

#endif // PURCHASESERVICE_H