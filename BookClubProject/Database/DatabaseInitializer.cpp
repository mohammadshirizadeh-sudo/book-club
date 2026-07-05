// DatabaseInitializer.cpp
#include "DatabaseInitializer.h"
#include "DatabaseManager.h"
#include <QDebug>

DatabaseInitializer::DatabaseInitializer(QObject *parent)
    : QObject(parent)
{
}

DatabaseInitializer::~DatabaseInitializer()
{
}

bool DatabaseInitializer::initialize(const QString& databasePath)
{
    m_databasePath = databasePath;

    // Open database
    if (!DatabaseManager::instance()->openDatabase(databasePath)) {
        qCritical() << "❌ Failed to open database for initialization";
        return false;
    }

    // Create tables
    if (!createTables()) {
        qCritical() << "❌ Failed to create tables";
        return false;
    }

    qDebug() << "✅ Database initialized successfully";
    return true;
}

bool DatabaseInitializer::createTables()
{
    qDebug() << "📋 Creating database tables...";

    return createUserTable() &&
           createBookTable() &&
           createReviewTable() &&
           createPurchaseTable() &&
           createPurchaseItemTable() &&
           createCartTable() &&
           createCartItemTable() &&
           createNotificationTable() &&
           createLibraryTable() &&
           createShelfTable() &&
           createShelfBookTable()&&
           createPublisherInfoTable() &&
           createAdminInfoTable();
}

bool DatabaseInitializer::dropTables()
{
    qDebug() << "🗑️ Dropping all tables...";

    // لیست کامل همه جداول
    QStringList tables = {
        // جداول Many-to-Many (اول)
        "shelf_book",
        "cart_item",
        "purchase_item",

        // جداول وابسته (second)
        "shelf",
        "library",
        "cart",
        "purchase",
        "review",
        "notification",
        "book",

        // اطلاعات اختصاصی (third)
        "publisher_info",    // ✅ وابسته به user
        "admin_info",        // ✅ وابسته به user

        // جدول اصلی (آخر)
        "user"
    };

    bool allSuccess = true;
    for (const QString& table : tables) {
        QString query = "DROP TABLE IF EXISTS " + table;
        if (!DatabaseManager::instance()->executeQuery(query)) {
            qWarning() << "⚠️ Failed to drop table:" << table;
            allSuccess = false;
        } else {
            qDebug() << "✅ Dropped table:" << table;
        }
    }

    if (allSuccess) {
        qDebug() << "✅ All tables dropped successfully!";
    } else {
        qWarning() << "⚠️ Some tables failed to drop!";
    }

    return allSuccess;
}

// =============================================
// ===== Create Tables =====
// =============================================

bool DatabaseInitializer::createUserTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS user (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            full_name TEXT,
            role TEXT NOT NULL DEFAULT 'User',
            status TEXT NOT NULL DEFAULT 'Active',
            favorite_genres TEXT,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            last_login TEXT,
            reset_token TEXT,
            reset_token_expiry TEXT
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}


bool DatabaseInitializer::createPublisherInfoTable() {
    QString query = R"(
        CREATE TABLE IF NOT EXISTS publisher_info (
            user_id INTEGER PRIMARY KEY,
            publisher_name TEXT NOT NULL,
            total_revenue REAL DEFAULT 0.0,
            joined_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}


bool DatabaseInitializer::createAdminInfoTable() {
    QString query = R"(
        CREATE TABLE IF NOT EXISTS admin_info (
            user_id INTEGER PRIMARY KEY,
            admin_level TEXT NOT NULL,
            last_action TEXT,
            FOREIGN KEY (user_id) REFERENCES user(id) ON DELETE CASCADE
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createBookTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS book (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            author TEXT NOT NULL,
            genre TEXT NOT NULL,
            description TEXT,
            price REAL NOT NULL,
            discount_percent REAL DEFAULT 0,
            cover_path TEXT,
            pdf_path TEXT,
            is_active INTEGER DEFAULT 1,
            average_rating REAL DEFAULT 0,
            sales_count INTEGER DEFAULT 0,
            publisher_id INTEGER NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (publisher_id) REFERENCES user(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createReviewTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS review (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            book_id INTEGER NOT NULL,
            text TEXT,
            rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id),
            FOREIGN KEY (book_id) REFERENCES book(id),
            UNIQUE(user_id, book_id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createPurchaseTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS purchase (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            total_price REAL NOT NULL,
            discount_amount REAL DEFAULT 0,
            final_price REAL NOT NULL,
            status TEXT NOT NULL DEFAULT 'Pending',
            purchased_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createPurchaseItemTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS purchase_item (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            purchase_id INTEGER NOT NULL,
            book_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL,
            unit_price REAL NOT NULL,
            discounted_price REAL NOT NULL,
            FOREIGN KEY (purchase_id) REFERENCES purchase(id),
            FOREIGN KEY (book_id) REFERENCES book(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createCartTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS cart (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL UNIQUE,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createCartItemTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS cart_item (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            cart_id INTEGER NOT NULL,
            book_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL,
            unit_price REAL NOT NULL,
            discounted_price REAL NOT NULL,
            FOREIGN KEY (cart_id) REFERENCES cart(id),
            FOREIGN KEY (book_id) REFERENCES book(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createNotificationTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS notification (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            type TEXT NOT NULL,
            title TEXT NOT NULL,
            message TEXT NOT NULL,
            is_read INTEGER DEFAULT 0,
            created_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createLibraryTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS library (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL UNIQUE,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES user(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createShelfTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS shelf (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            library_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (library_id) REFERENCES library(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}

bool DatabaseInitializer::createShelfBookTable()
{
    QString query = R"(
        CREATE TABLE IF NOT EXISTS shelf_book (
            shelf_id INTEGER NOT NULL,
            book_id INTEGER NOT NULL,
            added_at TEXT NOT NULL,
            PRIMARY KEY (shelf_id, book_id),
            FOREIGN KEY (shelf_id) REFERENCES shelf(id),
            FOREIGN KEY (book_id) REFERENCES book(id)
        )
    )";
    return DatabaseManager::instance()->executeQuery(query);
}