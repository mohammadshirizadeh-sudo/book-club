// cartservice.cpp
#include "CartService.h"
#include <QDebug>

// ===== Constructor =====
CartService::CartService(BookRepository* repo)
    : cart(nullptr)
    , bookRepo(repo) {
}

CartService::~CartService() {
    if (cart) {
        delete cart;
        cart = nullptr;
    }
}

// ===== Cart Management =====

void CartService::createCart(int userId) {
    if (cart) {
        delete cart;
        cart = nullptr;
    }
    cart = new Cart(userId);
    qDebug() << "Cart created for user:" << userId;
}

bool CartService::addToCart(int bookId, int quantity) {
    if (!cart) {
        qWarning() << "Cart not initialized! Call createCart() first.";
        return false;
    }

    if (quantity <= 0) {
        qWarning() << "Quantity must be positive!";
        return false;
    }

    // Get book from repository
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // Check if book is active
    if (!book->getIsActive()) {
        qWarning() << "Book is not active:" << book->getTitle();
        return false;
    }

    // Get book prices
    double unitPrice = book->getPrice();
    double discountedPrice = book->getFinalPrice();  // Price after discount

    // Create cart item
    CartItem item(bookId, quantity, unitPrice, discountedPrice);

    // Add to cart
    bool success = cart->addItem(item);
    if (success) {
        calculateTotal();
        qDebug() << "Added to cart:" << book->getTitle() << "x" << quantity;
    }

    return success;
}

bool CartService::removeFromCart(int bookId) {
    if (!cart) {
        qWarning() << "Cart not initialized!";
        return false;
    }

    bool success = cart->removeItem(bookId);
    if (success) {
        calculateTotal();
        qDebug() << "Removed from cart. Book ID:" << bookId;
    }

    return success;
}

bool CartService::updateQuantity(int bookId, int quantity) {
    if (!cart) {
        qWarning() << "Cart not initialized!";
        return false;
    }

    if (quantity < 0) {
        qWarning() << "Quantity cannot be negative!";
        return false;
    }

    // If quantity is 0, remove the item
    if (quantity == 0) {
        return removeFromCart(bookId);
    }

    // Get book to update prices (in case discount changed)
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // Update item in cart
    CartItem* item = cart->getItem(bookId);
    if (!item) {
        qWarning() << "Book not in cart:" << bookId;
        return false;
    }

    // Update quantity and prices
    item->setQuantity(quantity);
    item->setUnitPrice(book->getPrice());
    item->setDiscountedPrice(book->getFinalPrice());

    calculateTotal();
    qDebug() << "Updated quantity for book:" << bookId << "->" << quantity;
    return true;
}

void CartService::clearCart() {
    if (cart) {
        cart->clear();
        calculateTotal();
        qDebug() << "Cart cleared";
    }
}

// ===== Calculations =====

void CartService::calculateTotal() {
    if (!cart) {
        qWarning() << "Cart not initialized!";
        return;
    }
    cart->calculateTotals();
}

double CartService::getTotalPrice() const {
    if (!cart) return 0.0;
    return cart->getTotalPrice();
}

double CartService::getTotalDiscount() const {
    if (!cart) return 0.0;
    return cart->getTotalDiscount();
}

double CartService::getFinalPrice() const {
    if (!cart) return 0.0;
    return cart->getFinalPrice();
}

int CartService::getTotalItemCount() const {
    if (!cart) return 0;
    return cart->getTotalItems();
}

int CartService::getUniqueBookCount() const {
    if (!cart) return 0;
    return cart->getItemCount();
}

// ===== Getters =====

QVector<CartItem> CartService::getCartItems() const {
    if (!cart) return QVector<CartItem>();
    return cart->getItems();
}

bool CartService::isEmpty() const {
    if (!cart) return true;
    return cart->isEmpty();
}

bool CartService::contains(int bookId) const {
    if (!cart) return false;
    return cart->contains(bookId);
}

CartItem* CartService::getCartItem(int bookId) {
    if (!cart) return nullptr;
    return cart->getItem(bookId);
}

const CartItem* CartService::getCartItem(int bookId) const {
    if (!cart) return nullptr;
    return cart->getItem(bookId);
}

int CartService::getUserId() const {
    if (!cart) return -1;
    return cart->getUserId();
}

// ===== Private Methods =====

double CartService::getBookDiscountedPrice(int bookId) const {
    Book* book = bookRepo->findById(bookId);
    if (!book) return 0.0;
    return book->getFinalPrice();
}