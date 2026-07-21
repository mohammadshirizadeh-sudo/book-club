// LibraryService.cpp
#include "LibraryService.h"
#include <QDebug>

LibraryService::LibraryService(LibraryRepository* libraryRepo, QObject* parent)
    : QObject(parent)
    , m_libraryRepo(libraryRepo)
{
}

// =============================================
// ===== Library Management =====
// =============================================

QSharedPointer<Library> LibraryService::getLibraryByUserId(int userId) const
{
    if (userId <= 0) {
        qWarning() << "Invalid user ID:" << userId;
        return nullptr;
    }
    return m_libraryRepo->findByUserId(userId);
}

bool LibraryService::createLibrary(int userId)
{
    if (userId <= 0) {
        qWarning() << "Invalid user ID:" << userId;
        return false;
    }

    if (libraryExists(userId)) {
        qWarning() << "Library already exists for user:" << userId;
        return false;
    }

    QSharedPointer<Library> library = QSharedPointer<Library>::create(userId);
    return m_libraryRepo->addLibrary(library);
}

bool LibraryService::deleteLibrary(int userId)
{
    if (userId <= 0) {
        qWarning() << "Invalid user ID:" << userId;
        return false;
    }

    if (!libraryExists(userId)) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    return m_libraryRepo->deleteLibrary(userId);
}

bool LibraryService::libraryExists(int userId) const
{
    return m_libraryRepo->exists(userId);
}

// =============================================
// ===== Owned Books =====
// =============================================

bool LibraryService::addOwnedBook(int userId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (library->ownsBook(bookId)) {
        qWarning() << "User already owns book:" << bookId;
        return false;
    }

    library->addOwnedBook(bookId);
    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::removeOwnedBook(int userId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->ownsBook(bookId)) {
        qWarning() << "User does not own book:" << bookId;
        return false;
    }

    library->removeOwnedBook(bookId);
    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::ownsBook(int userId, int bookId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return false;
    }
    return library->ownsBook(bookId);
}

QVector<int> LibraryService::getOwnedBooks(int userId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return QVector<int>();
    }
    return library->getOwnedBooks();
}

// =============================================
// ===== Saved Books =====
// =============================================

bool LibraryService::saveBook(int userId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (library->isBookSaved(bookId)) {
        qWarning() << "Book already saved:" << bookId;
        return false;
    }

    library->saveBook(bookId);
    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::unsaveBook(int userId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->isBookSaved(bookId)) {
        qWarning() << "Book not saved:" << bookId;
        return false;
    }

    library->unsaveBook(bookId);
    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::isBookSaved(int userId, int bookId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return false;
    }
    return library->isBookSaved(bookId);
}

QVector<int> LibraryService::getSavedBooks(int userId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return QVector<int>();
    }
    return library->getSavedBooks();
}

// =============================================
// ===== Shelf Management =====
// =============================================

bool LibraryService::createShelf(int userId, const QString& shelfName)
{
    if (shelfName.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (getShelfByName(userId, shelfName)) {
        qWarning() << "Shelf with name" << shelfName << "already exists";
        return false;
    }

    int newShelfId = m_libraryRepo->insertShelf(userId, shelfName);
    if (newShelfId <= 0) {
        qWarning() << "Failed to insert shelf into database";
        return false;
    }

    // Use the DB-assigned id, not Library's own local-only id generator.
    return library->createShelf(newShelfId, shelfName);
}

bool LibraryService::deleteShelf(int userId, int shelfId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->getShelf(shelfId)) {
        qWarning() << "Shelf not found:" << shelfId;
        return false;
    }

    if (!m_libraryRepo->deleteShelfFromDb(shelfId)) {
        qWarning() << "Failed to delete shelf from database";
        return false;
    }

    return library->deleteShelf(shelfId);
}

bool LibraryService::renameShelf(int userId, int shelfId, const QString& newName)
{
    if (newName.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    QSharedPointer<Shelf> existing = getShelfByName(userId, newName);
    if (existing && existing->getShelfId() != shelfId) {
        qWarning() << "Shelf with name" << newName << "already exists";
        return false;
    }

    if (!m_libraryRepo->renameShelfInDb(shelfId, newName)) {
        qWarning() << "Failed to rename shelf in database";
        return false;
    }

    return library->renameShelf(shelfId, newName);
}

bool LibraryService::addBookToShelf(int userId, int shelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->ownsBook(bookId)) {
        qWarning() << "User does not own book:" << bookId;
        return false;
    }

    Shelf* shelf = library->getShelf(shelfId);
    if (!shelf) {
        qWarning() << "Shelf not found:" << shelfId;
        return false;
    }
    if (shelf->contains(bookId)) {
        qWarning() << "Book" << bookId << "already in shelf" << shelfId;
        return false;
    }

    if (!m_libraryRepo->addBookToShelfDb(shelfId, bookId)) {
        qWarning() << "Failed to add book to shelf in database";
        return false;
    }

    return library->addBookToShelf(shelfId, bookId);
}

bool LibraryService::removeBookFromShelf(int userId, int shelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!m_libraryRepo->removeBookFromShelfDb(shelfId, bookId)) {
        qWarning() << "Failed to remove book from shelf in database";
        return false;
    }

    return library->removeBookFromShelf(shelfId, bookId);
}

bool LibraryService::moveBookBetweenShelves(int userId, int fromShelfId, int toShelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->ownsBook(bookId)) {
        qWarning() << "User does not own book:" << bookId;
        return false;
    }

    if (!m_libraryRepo->moveBookBetweenShelvesDb(fromShelfId, toShelfId, bookId)) {
        qWarning() << "Failed to move book between shelves in database";
        return false;
    }

    if (!library->moveBookBetweenShelves(fromShelfId, toShelfId, bookId)) {
        qWarning() << "DB move succeeded but in-memory move failed — library cache is now stale for user:" << userId;
        return false;
    }

    return true;
}

QVector<Shelf> LibraryService::getShelves(int userId) const
{

    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return QVector<Shelf>();
    }

    return library->getShelves();
}




// LibraryService.cpp
QSharedPointer<Shelf> LibraryService::getShelfByName(int userId, const QString& name) const
{
    // 1. بررسی ورودی
    if (userId <= 0) {
        qWarning() << "Invalid user ID:" << userId;
        return nullptr;
    }

    if (name.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return nullptr;
    }

    // 2. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return nullptr;
    }

    // 3. جستجوی قفسه با نام
    const QVector<Shelf>& shelves = library->getShelves();
    for (const Shelf& shelf : shelves) {
        if (shelf.getName() == name) {
            return QSharedPointer<Shelf>::create(shelf);
        }
    }

    // 4. قفسه پیدا نشد
    qDebug() << "Shelf with name" << name << "not found for user" << userId;
    return nullptr;
}