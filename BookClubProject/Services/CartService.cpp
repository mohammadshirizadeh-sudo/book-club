// cartservice.cpp
#include "CartService.h"
#include <QDebug>

// ===== Constructor =====
CartService::CartService(BookRepository* repo , QObject* parent)
    : bookRepo(repo) , QObject(parent) {
    loadAllFromDatabase();
}

CartService::~CartService() {
    clearCache();
}

// ===== Cart Management =====

Cart* CartService::getOrcreateCart(int userId) {
    if (carts.contains(userId)) {
        return carts[userId];
    }
    Cart* newCart = new Cart(userId);
    carts[userId] = newCart;

    saveCartToDatabase(newCart);
    qDebug() << "Cart created for user:" << userId;
    return newCart;
}

bool CartService::addToCart(int userId, int bookId, int quantity) {
    Cart* cart = getOrcreateCart(userId);
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

        calculateTotal(userId);
        saveCartItemsToDatabase(userId, cart->getItems());
        qDebug() << "Added to cart:" << book->getTitle() << "x" << quantity;
    }

    return success;
}


bool CartService::removeFromCart(int userId, int bookId) {//بررسی کن منظور از این چیه



    if (!carts.contains(userId)) {
        qWarning() << "No cart found for user:" << userId;
        return false;
    }

    Cart* cart = carts[userId];
    bool success = cart->removeItem(bookId);
    if (success) {
        cart->calculateTotals();
        saveCartItemsToDatabase(userId, cart->getItems());
    }
    return success;
}

bool CartService::updateQuantity(int userId, int bookId, int quantity) {
    if (!carts.contains(userId)) {
        qWarning() << "No cart found for user:" << userId;
        return false;
    }
    Cart* cart = carts[userId];


    if (quantity < 0) {
        qWarning() << "Quantity cannot be negative!";
        return false;
    }


    // If quantity is 0, remove the item
    if (quantity == 0) {
        return removeFromCart(userId , bookId);
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

    calculateTotal(userId);
    saveCartItemsToDatabase(userId, cart->getItems());

    qDebug() << "Updated quantity for book:" << bookId << "->" << quantity;
    return true;
}



void CartService::clearCart(int userId) {
    if (carts.contains(userId)) {
        carts[userId]->clear();
        carts[userId]->calculateTotals();
        deleteCartFromDatabase(userId);
        qDebug() << "Cart cleared for user:" << userId;
    }
}

// ===== Calculations =====

void CartService::calculateTotal(int userId) {
    Cart* cart = carts[userId];
    if (!cart) {
        qWarning() << "Cart not initialized!";
        return;
    }
    cart->calculateTotals();
}

double CartService::getTotalPrice(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return 0.0;
    return cart->getTotalPrice();
}

double CartService::getTotalDiscount(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return 0.0;
    return cart->getTotalDiscount();
}

double CartService::getFinalPrice(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return 0.0;
    return cart->getFinalPrice();
}

int CartService::getTotalItemCount(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return 0;
    return cart->getTotalItems();
}

int CartService::getUniqueBookCount(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return 0;
    return cart->getItemCount();
}

// ===== Getters =====

QVector<CartItem> CartService::getCartItems(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return QVector<CartItem>();
    return cart->getItems();
}

bool CartService::isEmpty(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return true;
    return cart->isEmpty();
}

bool CartService::contains(int userId ,int bookId) const {
    Cart* cart = carts[userId];
    if (!cart) return false;
    return cart->contains(bookId);
}

CartItem* CartService::getCartItem(int userId ,int bookId) {
    Cart* cart = carts[userId];
    if (!cart) return nullptr;
    return cart->getItem(bookId);
}

const CartItem* CartService::getCartItem(int userId , int bookId) const {
    Cart* cart = carts[userId];
    if (!cart) return nullptr;
    return cart->getItem(bookId);
}

Cart *CartService::getCart(int userId) const
{

    return carts.value(userId , nullptr);
}

int CartService::getUserId(int userId) const {
    Cart* cart = carts[userId];
    if (!cart) return -1;
    return cart->getUserId();
}

// ===== Private Methods =====

double CartService::getBookDiscountedPrice(int bookId) const {
    Book* book = bookRepo->findById(bookId);
    if (!book) return 0.0;
    return book->getFinalPrice();
}




bool CartService::loadAllFromDatabase() {
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Load all carts
    QString query = R"(
        SELECT user_id, created_at, updated_at
        FROM cart
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load carts:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        int userId = sqlQuery.value("user_id").toInt();

        Cart* cart = new Cart(userId);
        cart->setCreatedAt(QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate));
        cart->setUpdatedAt(QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate));



        // Load cart items
        loadCartItems(cart);

        addToCache(cart);
        count++;
    }
    qDebug() << "✅ Loaded" << count << "carts from SQLite";
    return true;
}




bool CartService::saveCartToDatabase(Cart* cart) {
    if (!cart) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Using userId as cart id (1-to-1 relationship)
    QString query = R"(
        INSERT OR REPLACE INTO cart (
            id, user_id, created_at, updated_at
        ) VALUES (
            :id, :user_id, :created_at, :updated_at
        )
    )";

    QVariantMap params;
    params["id"] = cart->getUserId();
    params["user_id"] = cart->getUserId();
    params["created_at"] = cart->getCreatedAt().toString(Qt::ISODate);
    params["updated_at"] = cart->getUpdatedAt().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}

bool CartService::saveCartItemsToDatabase(int userId, const QVector<CartItem>& items) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Get cart id
    QString getCartIdQuery = "SELECT id FROM cart WHERE user_id = :user_id";
    QVariantMap cartParams;
    cartParams["user_id"] = userId;
    QSqlQuery cartQuery = db->executeSelect(getCartIdQuery, cartParams);

    if (!cartQuery.next()) {
        qWarning() << "Cart not found for user:" << userId;
        return false;
    }
    int cartId = cartQuery.value("id").toInt();

    // Delete existing items
    QString deleteQuery = "DELETE FROM cart_item WHERE cart_id = :cart_id";
    QVariantMap deleteParams;
    deleteParams["cart_id"] = cartId;
    db->executeQuery(deleteQuery, deleteParams);

    // Insert new items
    QString insertQuery = R"(
        INSERT INTO cart_item (
            cart_id, book_id, quantity, unit_price, discounted_price
        ) VALUES (
            :cart_id, :book_id, :quantity, :unit_price, :discounted_price
        )
    )";

    for (const CartItem& item : items) {
        QVariantMap params;
        params["cart_id"] = cartId;
        params["book_id"] = item.getBookId();
        params["quantity"] = item.getQuantity();
        params["unit_price"] = item.getUnitPrice();
        params["discounted_price"] = item.getDiscountedPrice();

        if (!db->executeQuery(insertQuery, params)) {
            return false;
        }
    }

    return true;
}

bool CartService::deleteCartFromDatabase(int userId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Get cart id
    QString getCartIdQuery = "SELECT id FROM cart WHERE user_id = :user_id";
    QVariantMap cartParams;
    cartParams["user_id"] = userId;
    QSqlQuery cartQuery = db->executeSelect(getCartIdQuery, cartParams);

    if (!cartQuery.next()) {
        return true;  // Cart doesn't exist
    }
    int cartId = cartQuery.value("id").toInt();

    // Delete cart items
    QString deleteItemsQuery = "DELETE FROM cart_item WHERE cart_id = :cart_id";
    QVariantMap deleteParams;
    deleteParams["cart_id"] = cartId;
    db->executeQuery(deleteItemsQuery, deleteParams);

    // Delete cart
    QString deleteCartQuery = "DELETE FROM cart WHERE id = :id";
    deleteParams.clear();
    deleteParams["id"] = cartId;
    return db->executeQuery(deleteCartQuery, deleteParams);
}

bool CartService::loadCartItems(Cart* cart) {
    if (!cart) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Get cart id
    QString getCartIdQuery = "SELECT id FROM cart WHERE user_id = :user_id";
    QVariantMap cartParams;
    cartParams["user_id"] = cart->getUserId();
    QSqlQuery cartQuery = db->executeSelect(getCartIdQuery, cartParams);

    if (!cartQuery.next()) {
        return true;  // Cart exists but no items
    }
    int cartId = cartQuery.value("id").toInt();

    // Load items
    QString query = R"(
        SELECT book_id, quantity, unit_price, discounted_price
        FROM cart_item
        WHERE cart_id = :cart_id
    )";

    QVariantMap params;
    params["cart_id"] = cartId;
    QSqlQuery sqlQuery = db->executeSelect(query, params);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load cart items:" << sqlQuery.lastError().text();
        return false;
    }

    QVector<CartItem> items;
    while (sqlQuery.next()) {
        CartItem item(
            sqlQuery.value("book_id").toInt(),
            sqlQuery.value("quantity").toInt(),
            sqlQuery.value("unit_price").toDouble(),
            sqlQuery.value("discounted_price").toDouble()
            );
        items.append(item);
    }

    cart->setItems(items);
    cart->calculateTotals();
    return true;
}



void CartService::addToCache(Cart* cart) {
    if (!cart) return;
    carts[cart->getUserId()] = cart;
}

void CartService::removeFromCache(int userId) {
    carts.remove(userId);
}

void CartService::clearCache() {
    qDeleteAll(carts);
    carts.clear();
}
