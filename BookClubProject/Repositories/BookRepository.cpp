// bookrepository.cpp
#include "BookRepository.h"
#include <QDebug>

BookRepository::BookRepository(QObject* parent) : QObject(parent) {
    loadAllFromDatabase();
}

BookRepository::~BookRepository() {
    clearCache();
}

bool BookRepository::addBook(Book* book) {
    if (!book) return false;

    int id = book->getBookId();

    // Check if book already exists
    if (booksById.contains(id)) {
        qWarning() << "Book with ID" << id << "already exists!";
        return false;
    }

    addToCache(book);

    // 2. Save to SQLite
    if (!saveToDatabase(book)) {
        removeFromCache(id);
        return false;
    }
    qDebug() << "Book added:" << book->getTitle();
    return true;
}

Book* BookRepository::findById(int id) const {
    return booksById.value(id, nullptr);
}

QVector<Book*> BookRepository::getAllBooks() const {
    return booksById.values().toVector();
}

bool BookRepository::updateBook(Book* book) {
    if (!book) return false;

    int id = book->getBookId();
    if (!booksById.contains(id)) {
        qWarning() << "Book with ID" << id << "not found!";
        return false;
    }


    booksById[id] = book;

    // 2. Update in SQLite
    if (!saveToDatabase(book)) {
        loadAllFromDatabase();
        return false;
    }
    return true;
}

bool BookRepository::deleteBook(int bookId) {
    Book* book = booksById.value(bookId, nullptr);
    if (!book) {
        qWarning() << "Book with ID" << bookId << "not found!";
        return false;
    }


    if (!deleteFromDatabase(bookId)) {
        qWarning() << "Failed to delete book from database!";
        return false;
    }

    // 2. Remove from cache
    removeFromCache(bookId);
    delete book;
    return true;
}



bool BookRepository::loadAllFromDatabase() {
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT id, title, author, genre, description, price,
               discount_percent, cover_path, pdf_path, is_active,
               average_rating, sales_count, publisher_id,
               created_at, updated_at
        FROM book
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load books:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        Book* book = new Book(
            sqlQuery.value("id").toInt(),
            sqlQuery.value("title").toString(),
            sqlQuery.value("author").toString(),
            GenreHelper::fromString(sqlQuery.value("genre").toString()),
            sqlQuery.value("description").toString(),
            sqlQuery.value("price").toDouble(),
            sqlQuery.value("discount_percent").toDouble(),
            sqlQuery.value("cover_path").toString(),
            sqlQuery.value("pdf_path").toString(),
            sqlQuery.value("is_active").toBool(),
            sqlQuery.value("average_rating").toDouble(),
            sqlQuery.value("sales_count").toInt(),
            sqlQuery.value("publisher_id").toInt(),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate),
            QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate)
            );

        addToCache(book);
        count++;
    }

    qDebug() << "✅ Loaded" << count << "books from SQLite";
    return true;
}

bool BookRepository::saveToDatabase(Book* book) {
    if (!book) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO book (
            id, title, author, genre, description, price,
            discount_percent, cover_path, pdf_path, is_active,
            average_rating, sales_count, publisher_id,
            created_at, updated_at
        ) VALUES (
            :id, :title, :author, :genre, :description, :price,
            :discount_percent, :cover_path, :pdf_path, :is_active,
            :average_rating, :sales_count, :publisher_id,
            :created_at, :updated_at
        )
    )";

    QVariantMap params;
    params["id"] = book->getBookId();
    params["title"] = book->getTitle();
    params["author"] = book->getAuthor();
    params["genre"] = GenreHelper::toString(book->getGenre());
    params["description"] = book->getDescription();
    params["price"] = book->getPrice();
    params["discount_percent"] = book->getDiscountPercent();
    params["cover_path"] = book->getCoverPath();
    params["pdf_path"] = book->getPdfPath();
    params["is_active"] = book->getIsActive() ? 1 : 0;
    params["average_rating"] = book->getAverageRating();
    params["sales_count"] = book->getSalesCount();
    params["publisher_id"] = book->getPublisherId();
    params["created_at"] = book->getCreatedAt().toString(Qt::ISODate);
    params["updated_at"] = book->getUpdatedAt().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}

bool BookRepository::deleteFromDatabase(int bookId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = "DELETE FROM book WHERE id = :id";
    QVariantMap params;
    params["id"] = bookId;

    return db->executeQuery(query, params);
}

// =============================================
// ===== Helper Methods =====
// =============================================

void BookRepository::addToCache(Book* book) {
    if (!book) return;
    booksById[book->getBookId()] = book;
}

void BookRepository::removeFromCache(int bookId) {
    booksById.remove(bookId);
}

void BookRepository::clearCache() {
    qDeleteAll(booksById);
    booksById.clear();
}


