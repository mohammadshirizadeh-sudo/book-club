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

bool LibraryRepository::addLibrary(QSharedPointer<Library> library) {

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
        QMutexLocker locker(&m_mutex);
        removeFromCache(userId);
        deleteFromDatabase(userId);
        qWarning() << "Failed to save shelves, rolled back library";
        return false;
    }
    int libraryId = getLibraryDbId(userId);

    if (libraryId <= 0 ||
        !saveOwnedBooks(libraryId, library->getOwnedBooks()) ||
        !saveSavedBooks(libraryId, library->getSavedBooks()))
    {

        QMutexLocker locker(&m_mutex);

        removeFromCache(userId);
        deleteFromDatabase(userId);


        qWarning()
            << "Failed to save owned/saved books, rolled back library";

        return false;
    }

    qDebug() << "Library added for user:" << userId;
    return true;
}

QSharedPointer<Library> LibraryRepository::findByUserId(int userId) const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.value(userId, nullptr);
}

QVector<QSharedPointer<Library>> LibraryRepository::getAllLibraries() const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.values().toVector();
}

bool LibraryRepository::updateLibrary(QSharedPointer<Library> library) {
    if (!library) {
        qWarning() << "Library is null!";
        return false;
    }

    int userId = library->getUserId();

    QSharedPointer<Library> oldLibrary = nullptr;

    {
        QMutexLocker locker(&m_mutex);

        if (!librariesByUserId.contains(userId)) {
            qWarning() << "Library for user" << userId << "not found!";
            return false;
        }

        oldLibrary = librariesByUserId[userId];
        librariesByUserId[userId] = library;
    }


    if (!saveToDatabase(library)) {
        QMutexLocker locker(&m_mutex);
        librariesByUserId[userId] = oldLibrary;
        return false;
    }


    // گرفتن id جدول library
    int libraryId = getLibraryDbId(userId);

    if (libraryId <= 0) {
        QMutexLocker locker(&m_mutex);
        librariesByUserId[userId] = oldLibrary;

        qWarning() << "Could not resolve library DB id for user:" << userId;
        return false;
    }

    /*

    if (!saveShelves(userId, library->getShelves())) {
        QMutexLocker locker(&m_mutex);
        librariesByUserId[userId] = oldLibrary;
        return false;
    }
*/


    if (!saveOwnedBooks(libraryId, library->getOwnedBooks())) {
        QMutexLocker locker(&m_mutex);
        librariesByUserId[userId] = oldLibrary;

        qWarning() << "Failed to save owned books, rolled back";
        return false;
    }


    if (!saveSavedBooks(libraryId, library->getSavedBooks())) {
        QMutexLocker locker(&m_mutex);
        librariesByUserId[userId] = oldLibrary;

        qWarning() << "Failed to save saved books, rolled back";
        return false;
    }


    qDebug() << "Library updated for user:" << userId;
    return true;
}



int LibraryRepository::getLibraryDbId(int userId) const {

    DatabaseManager* db = DatabaseManager::instance();

    QString query =
        "SELECT id FROM library WHERE user_id = :user_id";


    QVariantMap params;
    params["user_id"] = userId;


    QSqlQuery sqlQuery =
        db->executeSelect(query, params);


    if (!sqlQuery.next())
        return -1;


    return sqlQuery.value("id").toInt();
}
bool LibraryRepository::deleteLibrary(int userId) {
    QMutexLocker locker(&m_mutex);
    QSharedPointer<Library> library = librariesByUserId.value(userId, nullptr);
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
    qDebug() << "Library deleted for user:" << userId;
    return true;
}

bool LibraryRepository::exists(int userId) const {
     QMutexLocker locker(&m_mutex);
    return librariesByUserId.contains(userId);
}


bool LibraryRepository::loadAllFromDatabase()
{
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
        int libraryId = sqlQuery.value("id").toInt();
        int userId = sqlQuery.value("user_id").toInt();

        QSharedPointer<Library> library = QSharedPointer<Library>::create(
            userId,
            QVector<int>(),
            QVector<int>(),
            QVector<Shelf>(),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate),
            QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate)
            );

        // ✅ بارگذاری ownedBooks و savedBooks
        loadOwnedBooks(library, libraryId);
        loadSavedBooks(library, libraryId);

        // بارگذاری shelves
        loadShelves(library);

        addToCache(library);
        count++;
    }

    qDebug() << "✅ Loaded" << count << "libraries from SQLite";
    return true;
}


/*
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
        QSharedPointer<Library> library = QSharedPointer<Library>::create(
            sqlQuery.value("user_id").toInt(),
            QVector<int>(),
            QVector<int>(),
            QVector<Shelf>()
            );


        loadShelves(library);

        addToCache(library);
        count++;
    }
    qDebug() << "✅ Loaded" << count << "libraries from SQLite";
    return true;
}
*/
/*
bool LibraryRepository::saveToDatabase(QSharedPointer<Library> library) {
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
*/
bool LibraryRepository::saveToDatabase(QSharedPointer<Library> library) {
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

    int libId = library->getUserId();
    int userId = library->getUserId();

    QVariantMap params;
    params["id"] = libId;
    params["user_id"] = userId;
    params["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    params["updated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

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

bool LibraryRepository::saveShelves(int userId, const QVector<Shelf>& shelves)
{
    // Used only for one-time bulk population of a brand-new library in
    // addLibrary(). Incremental shelf changes go through insertShelf(),
    // renameShelfInDb(), deleteShelfFromDb(), addBookToShelfDb(),
    // removeBookFromShelfDb(), and moveBookBetweenShelvesDb() instead —
    // none of those ever wipe the whole shelf table.
    if (shelves.isEmpty()) {
        return true;
    }

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    int libDbId = getLibraryDbId(userId);
    if (libDbId <= 0) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!db->transaction()) {
        qWarning() << "Failed to start transaction";
        return false;
    }

    bool ok = true;
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

    if (ok && db->commit()) {
        return true;
    }

    db->rollback();
    qWarning() << "Failed to save initial shelves for user:" << userId;
    return false;
}


/*
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

    QString deleteShelfBooksQuery = R"(
    DELETE FROM shelf_book
    WHERE shelf_id IN (
        SELECT id FROM shelf
        WHERE library_id = :library_id
    )
)";
    QVariantMap deleteShelfBooksParams;
    deleteShelfBooksParams["library_id"] = libDbId;
    ok = db->executeQuery(deleteShelfBooksQuery, deleteShelfBooksParams);

    QString deleteShelvesQuery =
        "DELETE FROM shelf WHERE library_id = :library_id";

    QVariantMap deleteParams;
    deleteParams["library_id"] = libDbId;

    if (ok) {
        ok = db->executeQuery(deleteShelvesQuery, deleteParams);
    }

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
*/
bool LibraryRepository::loadShelves(QSharedPointer<Library> library) {
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

void LibraryRepository::addToCache(QSharedPointer<Library> library) {

    if (!library) return;
    librariesByUserId[library->getUserId()] = library;
}

void LibraryRepository::removeFromCache(int userId) {
    librariesByUserId.remove(userId);
}

void LibraryRepository::clearCache() {
    librariesByUserId.clear();
}


// LibraryRepository.cpp

// =============================================
// ===== Owned Books =====
// =============================================

bool LibraryRepository::saveOwnedBooks(int libraryId, const QVector<int>& ownedBooks)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // 1. حذف رکوردهای قبلی
    QString deleteQuery = "DELETE FROM library_owned_book WHERE library_id = :library_id";
    QVariantMap deleteParams;
    deleteParams["library_id"] = libraryId;
    if (!db->executeQuery(deleteQuery, deleteParams)) {
        qWarning() << "Failed to delete existing owned books";
        return false;
    }

    // 2. درج رکوردهای جدید
    if (ownedBooks.isEmpty()) {
        return true;
    }

    QString insertQuery = R"(
        INSERT INTO library_owned_book (library_id, book_id, added_at)
        VALUES (:library_id, :book_id, :added_at)
    )";

    for (int bookId : ownedBooks) {
        QVariantMap params;
        params["library_id"] = libraryId;
        params["book_id"] = bookId;
        params["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        if (!db->executeQuery(insertQuery, params)) {
            qWarning() << "Failed to insert owned book:" << bookId;
            return false;
        }
    }

    qDebug() << "✅ Saved" << ownedBooks.size() << "owned books for library" << libraryId;
    return true;
}

bool LibraryRepository::loadOwnedBooks(QSharedPointer<Library> library, int libraryId)
{
    if (!library) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT book_id FROM library_owned_book
        WHERE library_id = :library_id
        ORDER BY added_at DESC
    )";

    QVariantMap params;
    params["library_id"] = libraryId;

    QSqlQuery sqlQuery = db->executeSelect(query, params);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load owned books:" << sqlQuery.lastError().text();
        return false;
    }

    QVector<int> ownedBooks;
    while (sqlQuery.next()) {
        ownedBooks.append(sqlQuery.value("book_id").toInt());
    }

    library->setOwnedBooks(ownedBooks);
    qDebug() << "✅ Loaded" << ownedBooks.size() << "owned books for library" << libraryId;
    return true;
}

// =============================================
// ===== Saved Books =====
// =============================================

bool LibraryRepository::saveSavedBooks(int libraryId, const QVector<int>& savedBooks)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // 1. حذف رکوردهای قبلی
    QString deleteQuery = "DELETE FROM library_saved_book WHERE library_id = :library_id";
    QVariantMap deleteParams;
    deleteParams["library_id"] = libraryId;
    if (!db->executeQuery(deleteQuery, deleteParams)) {
        qWarning() << "Failed to delete existing saved books";
        return false;
    }

    // 2. درج رکوردهای جدید
    if (savedBooks.isEmpty()) {
        return true;
    }

    QString insertQuery = R"(
        INSERT INTO library_saved_book (library_id, book_id, saved_at)
        VALUES (:library_id, :book_id, :saved_at)
    )";

    for (int bookId : savedBooks) {
        QVariantMap params;
        params["library_id"] = libraryId;
        params["book_id"] = bookId;
        params["saved_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        if (!db->executeQuery(insertQuery, params)) {
            qWarning() << "Failed to insert saved book:" << bookId;
            return false;
        }
    }

    qDebug() << "✅ Saved" << savedBooks.size() << "saved books for library" << libraryId;
    return true;
}

bool LibraryRepository::loadSavedBooks(QSharedPointer<Library> library, int libraryId)
{
    if (!library) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT book_id FROM library_saved_book
        WHERE library_id = :library_id
        ORDER BY saved_at DESC
    )";

    QVariantMap params;
    params["library_id"] = libraryId;

    QSqlQuery sqlQuery = db->executeSelect(query, params);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load saved books:" << sqlQuery.lastError().text();
        return false;
    }

    QVector<int> savedBooks;
    while (sqlQuery.next()) {
        savedBooks.append(sqlQuery.value("book_id").toInt());
    }

    library->setSavedBooks(savedBooks);
    qDebug() << "✅ Loaded" << savedBooks.size() << "saved books for library" << libraryId;
    return true;
}


int LibraryRepository::insertShelf(int userId, const QString& name)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return -1;
    }

    int libDbId = getLibraryDbId(userId);
    if (libDbId <= 0) {
        qWarning() << "Library not found for user:" << userId;
        return -1;
    }

    if (!db->transaction()) {
        qWarning() << "Failed to start transaction";
        return -1;
    }

    // shelf.id is a global PRIMARY KEY (not scoped per-user), so we must
    // hand out a globally-unique id here rather than trust a caller-supplied
    // one — Library::createShelf(name) only guarantees uniqueness within
    // that one user's own shelf list, which collides across users.
    QSqlQuery maxQuery = db->executeSelect("SELECT COALESCE(MAX(id), 0) + 1 AS next_id FROM shelf");
    int newId = 1;
    if (maxQuery.next()) {
        newId = maxQuery.value("next_id").toInt();
    }

    QString insertShelfQuery = R"(
        INSERT INTO shelf (id, library_id, name, created_at, updated_at)
        VALUES (:id, :library_id, :name, :created_at, :updated_at)
    )";
    QVariantMap params;
    params["id"] = newId;
    params["library_id"] = libDbId;
    params["name"] = name;
    params["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    params["updated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!db->executeQuery(insertShelfQuery, params)) {
        db->rollback();
        qWarning() << "Failed to insert shelf for user:" << userId;
        return -1;
    }

    if (!db->commit()) {
        db->rollback();
        qWarning() << "Commit failed while inserting shelf";
        return -1;
    }

    return newId;
}



bool LibraryRepository::deleteShelfFromDb(int shelfId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    if (!db->transaction()) {
        qWarning() << "Failed to start transaction";
        return false;
    }

    QVariantMap params;
    params["shelf_id"] = shelfId;

    if (!db->executeQuery("DELETE FROM shelf_book WHERE shelf_id = :shelf_id", params)) {
        db->rollback();
        qWarning() << "Failed to delete shelf_book rows for shelf:" << shelfId;
        return false;
    }

    if (!db->executeQuery("DELETE FROM shelf WHERE id = :shelf_id", params)) {
        db->rollback();
        qWarning() << "Failed to delete shelf:" << shelfId;
        return false;
    }

    if (!db->commit()) {
        db->rollback();
        qWarning() << "Commit failed while deleting shelf";
        return false;
    }

    return true;
}



bool LibraryRepository::addBookToShelfDb(int shelfId, int bookId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // INSERT OR IGNORE: (shelf_id, book_id) is the primary key, so this is a
    // no-op if it somehow already exists rather than a hard failure.
    QString query = R"(
        INSERT OR IGNORE INTO shelf_book (shelf_id, book_id, added_at)
        VALUES (:shelf_id, :book_id, :added_at)
    )";
    QVariantMap params;
    params["shelf_id"] = shelfId;
    params["book_id"] = bookId;
    params["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}

bool LibraryRepository::removeBookFromShelfDb(int shelfId, int bookId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = "DELETE FROM shelf_book WHERE shelf_id = :shelf_id AND book_id = :book_id";
    QVariantMap params;
    params["shelf_id"] = shelfId;
    params["book_id"] = bookId;

    return db->executeQuery(query, params);
}


bool LibraryRepository::moveBookBetweenShelvesDb(int fromShelfId, int toShelfId, int bookId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    if (!db->transaction()) {
        qWarning() << "Failed to start transaction";
        return false;
    }

    QVariantMap deleteParams;
    deleteParams["shelf_id"] = fromShelfId;
    deleteParams["book_id"] = bookId;
    if (!db->executeQuery("DELETE FROM shelf_book WHERE shelf_id = :shelf_id AND book_id = :book_id", deleteParams)) {
        db->rollback();
        qWarning() << "Failed to remove book from source shelf during move";
        return false;
    }

    QVariantMap insertParams;
    insertParams["shelf_id"] = toShelfId;
    insertParams["book_id"] = bookId;
    insertParams["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString insertQuery = R"(
        INSERT OR IGNORE INTO shelf_book (shelf_id, book_id, added_at)
        VALUES (:shelf_id, :book_id, :added_at)
    )";
    if (!db->executeQuery(insertQuery, insertParams)) {
        db->rollback();
        qWarning() << "Failed to add book to destination shelf during move";
        return false;
    }

    if (!db->commit()) {
        db->rollback();
        qWarning() << "Commit failed during move";
        return false;
    }

    return true;
}
// LibraryRepository.cpp
bool LibraryRepository::renameShelfInDb(int shelfId, const QString& newName)
{
    // 1. اعتبارسنجی
    if (shelfId <= 0) {
        qWarning() << "Invalid shelf ID:" << shelfId;
        return false;
    }

    if (newName.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    // 2. دریافت اتصال دیتابیس
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    // 3. به‌روزرسانی نام قفسه
    QString query = R"(
        UPDATE shelf
        SET name = :name, updated_at = :updated_at
        WHERE id = :shelf_id
    )";

    QVariantMap params;
    params["name"] = newName;
    params["updated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    params["shelf_id"] = shelfId;

    bool success = db->executeQuery(query, params);

    if (success) {
        qDebug() << "✅ Shelf renamed successfully:" << shelfId << "->" << newName;
    } else {
        qWarning() << "❌ Failed to rename shelf:" << shelfId;
    }

    return success;
}
