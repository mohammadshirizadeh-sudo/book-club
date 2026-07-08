// libraryrepository.cpp
#include "LibraryRepository.h"
#include <QDebug>
#include <qdatetime.h>

LibraryRepository::LibraryRepository(QObject* parent):QObject(parent) {
    loadAllFromDatabase();
}

LibraryRepository::~LibraryRepository() {
    clearCache();
}

bool LibraryRepository::addLibrary(Library* library) {

    if (!library) {
        qWarning() << "Library is null!";
        return false;
    }

    int userId = library->getUserId();

    {
        QMutexLocker locker(&m_mutex);
        // Check if library already exists
        if (librariesByUserId.contains(userId)) {
            qWarning() << "Library for user" << userId << "already exists!";
            return false;
        }

        addToCache(library);

    }


    // 2. Save to SQLite
    if (!saveToDatabase(library)) {
        QMutexLocker locker(&m_mutex);
        removeFromCache(userId);
        return false;
    }

    // 3. Save shelves
    if (!saveShelves(userId, library->getShelves())) {
        qWarning() << "Failed to save shelves!";
        return false;
    }

    qDebug() << "Library added for user:" << userId;
    return true;
}

Library* LibraryRepository::findByUserId(int userId) const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.value(userId, nullptr);
}

QVector<Library*> LibraryRepository::getAllLibraries() const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.values().toVector();
}

bool LibraryRepository::updateLibrary(Library* library) {

    if (!library) {
        qWarning() << "Library is null!";
        return false;
    }

    int userId = library->getUserId();
    {
        QMutexLocker locker(&m_mutex);
        if (!librariesByUserId.contains(userId)) {
            qWarning() << "Library for user" << userId << "not found!";
            return false;
        }


        librariesByUserId[userId] = library;


    }

    if (!saveToDatabase(library)) {
        QMutexLocker locker(&m_mutex);

        removeFromCache(userId);
        qWarning() << "Failed to update library in database, removed from cache";
        return false;
    }

    // 3. Update shelves
    if (!saveShelves(userId, library->getShelves())) {
        QMutexLocker locker(&m_mutex);
        removeFromCache(userId);
        qWarning() << "Failed to save shelves!";
        return false;
    }

    qDebug() << "Library updated for user:" << userId;
    return true;
}

bool LibraryRepository::deleteLibrary(int userId) {
    QMutexLocker locker(&m_mutex);
    Library* library = librariesByUserId.value(userId, nullptr);
    if (!library) {
        qWarning() << "Library for user" << userId << "not found!";
        return false;
    }


    if (!deleteFromDatabase(userId)) {
        qWarning() << "Failed to delete library from database!";
        return false;
    }

    // 2. Remove from cache
    removeFromCache(userId);
    delete library;
    qDebug() << "Library deleted for user:" << userId;
    return true;
}

bool LibraryRepository::exists(int userId) const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.contains(userId);
}


bool LibraryRepository::loadAllFromDatabase() {
    QMutexLocker locker(&m_mutex);
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Load libraries
    QString query = R"(
        SELECT id, user_id, created_at, updated_at
        FROM library
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load libraries:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        Library* library = new Library(
            sqlQuery.value("user_id").toInt(),
            QVector<int>(),  // ownedBooks - loaded separately
            QVector<int>(),  // savedBooks - loaded separately
            QVector<Shelf>() // shelves - loaded separately
            );

        // Load shelves
        loadShelves(library);

        addToCache(library);
        count++;
    }
    qDebug() << "✅ Loaded" << count << "libraries from SQLite";
    return true;
}

bool LibraryRepository::saveToDatabase(Library* library) {
    if (!library) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO library (
            id, user_id, created_at, updated_at
        ) VALUES (
            :id, :user_id, :created_at, :updated_at
        )
    )";

    // Using userId as library id (1-to-1 relationship)
    int libId = library->getUserId();
    int userId = library->getUserId();

    QVariantMap params;
    params["id"] = libId;
    params["user_id"] = userId;

    return db->executeQuery(query, params);
}

bool LibraryRepository::deleteFromDatabase(int userId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // 1. Delete shelves and shelf_books first (cascade)
    QString deleteShelfBooksQuery = R"(
        DELETE FROM shelf_book
        WHERE shelf_id IN (SELECT id FROM shelf WHERE library_id = (SELECT id FROM library WHERE user_id = :user_id))
    )";
    QVariantMap params;
    params["user_id"] = userId;
    db->executeQuery(deleteShelfBooksQuery, params);

    // 2. Delete shelves
    QString deleteShelvesQuery = "DELETE FROM shelf WHERE library_id = (SELECT id FROM library WHERE user_id = :user_id)";
    db->executeQuery(deleteShelvesQuery, params);

    // 3. Delete library
    QString deleteLibraryQuery = "DELETE FROM library WHERE user_id = :user_id";
    return db->executeQuery(deleteLibraryQuery, params);
}

bool LibraryRepository::saveShelves(int userId, const QVector<Shelf>& shelves) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }


    if (!db->transaction()) {
        qWarning() << "Failed to start transaction";
        return false;
    }

    bool ok = true;

    QString getLibIdQuery = "SELECT id FROM library WHERE user_id = :user_id";
    QVariantMap libParams;
    libParams["user_id"] = userId;
    QSqlQuery libQuery = db->executeSelect(getLibIdQuery, libParams);

    if (!libQuery.next()) {
        qWarning() << "Library not found for user:" << userId;
        db->rollback();
        return false;
    }
    int libDbId = libQuery.value("id").toInt();

    QString deleteShelvesQuery = "DELETE FROM shelf WHERE library_id = :library_id";
    QVariantMap deleteParams;
    deleteParams["library_id"] = libDbId;
    ok = db->executeQuery(deleteShelvesQuery, deleteParams);

    if (ok) {
        QString insertShelfQuery = R"(
            INSERT INTO shelf (id, library_id, name, created_at, updated_at)
            VALUES (:id, :library_id, :name, :created_at, :updated_at)
        )";

        QString insertShelfBookQuery = R"(
            INSERT INTO shelf_book (shelf_id, book_id, added_at)
            VALUES (:shelf_id, :book_id, :added_at)
        )";

        for (const Shelf& shelf : shelves) {
            QVariantMap shelfParams;
            shelfParams["id"] = shelf.getShelfId();
            shelfParams["library_id"] = libDbId;
            shelfParams["name"] = shelf.getName();
            shelfParams["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            shelfParams["updated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

            ok = db->executeQuery(insertShelfQuery, shelfParams);
            if (!ok) break;

            for (int bookId : shelf.getBookIds()) {
                QVariantMap bookParams;
                bookParams["shelf_id"] = shelf.getShelfId();
                bookParams["book_id"] = bookId;
                bookParams["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

                ok = db->executeQuery(insertShelfBookQuery, bookParams);
                if (!ok) break;
            }
            if (!ok) break;
        }
    }

    // ✅ 2. Commit یا Rollback
    if (ok) {
        if (!db->commit()) {
            qWarning() << "Commit failed!";
            db->rollback();
            return false;
        }
        qDebug() << "✅ Shelves saved successfully for user:" << userId;
        return true;
    } else {
        db->rollback();
        qWarning() << "❌ Shelves save failed, rolling back";
        return false;
    }
}
bool LibraryRepository::loadShelves(Library* library) {
    if (!library) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // Get library database id
    QString getLibIdQuery = "SELECT id FROM library WHERE user_id = :user_id";
    QVariantMap libParams;
    libParams["user_id"] = library->getUserId();
    QSqlQuery libQuery = db->executeSelect(getLibIdQuery, libParams);

    if (!libQuery.next()) {
        qWarning() << "Library not found for user:" << library->getUserId();
        return false;
    }
    int libDbId = libQuery.value("id").toInt();

    // Load shelves
    QString query = R"(
        SELECT id, name
        FROM shelf
        WHERE library_id = :library_id
    )";

    QVariantMap params;
    params["library_id"] = libDbId;
    QSqlQuery sqlQuery = db->executeSelect(query, params);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load shelves:" << sqlQuery.lastError().text();
        return false;
    }

    QVector<Shelf> shelves;
    while (sqlQuery.next()) {
        int shelfId = sqlQuery.value("id").toInt();
        QString name = sqlQuery.value("name").toString();

        // Load books in this shelf
        QString bookQuery = R"(
            SELECT book_id
            FROM shelf_book
            WHERE shelf_id = :shelf_id
        )";

        QVariantMap bookParams;
        bookParams["shelf_id"] = shelfId;
        QSqlQuery bookSqlQuery = db->executeSelect(bookQuery, bookParams);

        QVector<int> bookIds;
        while (bookSqlQuery.next()) {
            bookIds.append(bookSqlQuery.value("book_id").toInt());
        }

        Shelf shelf(shelfId, name);
        for (int bookId : bookIds) {
            shelf.addBook(bookId);
        }
        shelves.append(shelf);
    }

    library->setShelves(shelves);
    return true;
}

// =============================================
// ===== Helper Methods =====
// =============================================

void LibraryRepository::addToCache(Library* library) {

    if (!library) return;
    librariesByUserId[library->getUserId()] = library;
}

void LibraryRepository::removeFromCache(int userId) {
    librariesByUserId.remove(userId);
}

void LibraryRepository::clearCache() {
    qDeleteAll(librariesByUserId);
    librariesByUserId.clear();
}
