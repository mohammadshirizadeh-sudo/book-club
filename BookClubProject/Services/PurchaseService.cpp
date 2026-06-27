// purchaseservice.cpp
#include "Purchaseservice.h"
#include <QDebug>

// ===== Constructor =====
PurchaseService::PurchaseService(PurchaseRepository* purchaseRepo,
                                 BookRepository* bookRepo,
                                 LibraryRepository* libraryRepo,
                                 CartService* cartService,
                                 NotificationService* notifService)
    : purchaseRepo(purchaseRepo)
    , bookRepo(bookRepo)
    , libraryRepo(libraryRepo)
    , cartService(cartService)
    , notifService(notifService) {
}

// ===== Purchase Operations =====

Purchase* PurchaseService::checkout(int userId) {
    // 1. Get current cart
    Cart* cart = cartService->getCart(userId);
    if (!cart || cart->isEmpty()) {
        qWarning() << "Cart is empty or not initialized!";
        return nullptr;
    }

    // 2. Verify cart belongs to user
    if (cart->getUserId() != userId) {
        qWarning() << "Cart does not belong to user:" << userId;
        return nullptr;
    }

    // 3. Get cart data
    QVector<CartItem> cartItems = cart->getItems();
    double totalPrice = cart->getTotalPrice();
    double discountAmount = cart->getTotalDiscount();
    double finalPrice = cart->getFinalPrice();

    // 4. Process payment
    if (!processPayment(finalPrice)) {
        qWarning() << "Payment failed for user:" << userId;
        return nullptr;
    }

    // 5. Create purchase
    Purchase* purchase = createPurchase(userId, cartItems, totalPrice, discountAmount, finalPrice);
    if (!purchase) {
        qWarning() << "Failed to create purchase for user:" << userId;
        return nullptr;
    }

    // 6. Move books to library
    if (!moveToLibrary(userId, cartItems)) {
        qWarning() << "Failed to move books to library for user:" << userId;
        // Still return purchase, but log error
    }

    // 7. Update book sales counts
    updateBookSales(cartItems);

    // 8. Send notifications
    sendPurchaseConfirmation(userId, purchase);

    // 9. Clear the cart
    cartService->clearCart(userId);

    qDebug() << "Checkout completed for user:" << userId
             << "Purchase ID:" << purchase->getPurchaseId()
             << "Final price:" << finalPrice;

    return purchase;
}

Purchase* PurchaseService::createPurchase(int userId, const QVector<CartItem>& cartItems,
                                          double totalPrice, double discountAmount,
                                          double finalPrice) {
    // 1. Generate new purchase ID
    int purchaseId = nextPurchaseId++;

    // 2. Create purchase
    Purchase* purchase = new Purchase(purchaseId, userId, cartItems,
                                      totalPrice, discountAmount, finalPrice);

    // 3. Save to repository
    if (!purchaseRepo->addPurchase(purchase)) {
        delete purchase;
        qWarning() << "Failed to save purchase to repository!";
        return nullptr;
    }

    qDebug() << "Purchase created:" << purchaseId << "for user:" << userId;
    return purchase;
}

bool PurchaseService::moveToLibrary(int userId, const QVector<CartItem>& purchaseItems) {
    // 1. Get user's library
    Library* library = libraryRepo->findByUserId(userId);
    if (!library) {
        // If library doesn't exist, create one
        library = new Library(userId);
        libraryRepo->addLibrary(library);
    }

    // 2. Add each purchased book to library
    bool allAdded = true;
    for (const CartItem& item : purchaseItems) {
        int bookId = item.getBookId();

        // Only add once (if user already owns it, skip or handle)
        if (!library->ownsBook(bookId)) {
            if (!library->addOwnedBook(bookId)) {
                qWarning() << "Failed to add book" << bookId << "to library";
                allAdded = false;
            }
        }
    }

    // 3. Update library in repository
    if (!libraryRepo->updateLibrary(library)) {
        qWarning() << "Failed to update library for user:" << userId;
        return false;
    }

    qDebug() << "Moved" << purchaseItems.size() << "books to library for user:" << userId;
    return allAdded;
}

// ===== Purchase History =====

QVector<Purchase*> PurchaseService::getPurchaseHistory(int userId) const {
    return purchaseRepo->getPurchasesByUserId(userId);
}

Purchase* PurchaseService::getPurchaseById(int purchaseId) const {
    return purchaseRepo->findById(purchaseId);
}

QVector<Purchase*> PurchaseService::getAllPurchases() const {
    return purchaseRepo->getAllPurchases();
}

QVector<Purchase*> PurchaseService::getPurchasesForBook(int bookId) const {
    return purchaseRepo->getPurchasesByBookId(bookId);
}

// ===== Status Management =====

bool PurchaseService::updatePurchaseStatus(int purchaseId, PurchaseStatus newStatus) {
    Purchase* purchase = purchaseRepo->findById(purchaseId);
    if (!purchase) {
        qWarning() << "Purchase not found:" << purchaseId;
        return false;
    }

    purchase->setStatus(newStatus);
    return purchaseRepo->updatePurchase(purchase);
}

bool PurchaseService::cancelPurchase(int purchaseId) {
    Purchase* purchase = purchaseRepo->findById(purchaseId);
    if (!purchase) {
        qWarning() << "Purchase not found:" << purchaseId;
        return false;
    }

    if (purchase->isCompleted()) {
        qWarning() << "Cannot cancel a completed purchase!";
        return false;
    }

    purchase->setStatus(PurchaseStatus::Cancelled);
    return purchaseRepo->updatePurchase(purchase);
}

bool PurchaseService::refundPurchase(int purchaseId) {
    Purchase* purchase = purchaseRepo->findById(purchaseId);
    if (!purchase) {
        qWarning() << "Purchase not found:" << purchaseId;
        return false;
    }

    if (!purchase->isCompleted()) {
        qWarning() << "Can only refund completed purchases!";
        return false;
    }

    purchase->setStatus(PurchaseStatus::Refunded);
    return purchaseRepo->updatePurchase(purchase);
}

// ===== Private Methods =====

bool PurchaseService::processPayment(double amount) {
    // Simulate payment processing
    // In a real application, this would connect to a payment gateway

    if (amount < 0) {
        qWarning() << "Invalid payment amount:" << amount;
        return false;
    }

    // Simulate successful payment (always succeeds for demo)
    qDebug() << "Payment processed successfully. Amount:" << amount;
    return true;
}

void PurchaseService::updateBookSales(const QVector<CartItem>& purchaseItems) {
    for (const CartItem& item : purchaseItems) {
        int bookId = item.getBookId();
        int quantity = item.getQuantity();

        Book* book = bookRepo->findById(bookId);
        if (book) {
            int newSales = book->getSalesCount() + quantity;
            book->setSalesCount(newSales);
            bookRepo->updateBook(book);

            qDebug() << "Updated sales for book" << book->getTitle()
                     << ":" << quantity << "new sales";
        }
    }
}

void PurchaseService::sendPurchaseConfirmation(int userId, Purchase* purchase) {
    if (!notifService) return;

    QString title = "✅ Purchase Confirmed!";
    QString message = QString("Your purchase of %1 items was successful! Total: $%2")
                          .arg(purchase->getTotalItemCount())
                          .arg(purchase->getFinalPrice(), 0, 'f', 2);

    notifService->sendToUser(userId, NotificationType::Purchase, title, message);

    // Notify publishers about their book sales
    for (const CartItem& item : purchase->getItems()) {
        Book* book = bookRepo->findById(item.getBookId());
        if (book) {
            QString publisherMsg = QString("Your book '%1' was sold! (x%2)")
            .arg(book->getTitle())
                .arg(item.getQuantity());
            notifService->sendToRole("Publisher", NotificationType::NewSale,
                                     "💰 New Sale!", publisherMsg);
        }
    }
}