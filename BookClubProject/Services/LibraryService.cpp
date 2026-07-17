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

    // بررسی وجود قفسه با همین نام
    if (getShelfByName(userId, shelfName)) {
        qWarning() << "Shelf with name" << shelfName << "already exists";
        return false;
    }




    library->createShelf(shelfName);

    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::deleteShelf(int userId, int shelfId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->deleteShelf(shelfId)) {
        qWarning() << "Shelf not found:" << shelfId;
        return false;
    }

    return m_libraryRepo->updateLibrary(library);
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

    // بررسی اینکه نام جدید توسط قفسه دیگری استفاده نشده باشد
    QSharedPointer<Shelf> existing = getShelfByName(userId, newName);
    if (existing && existing->getShelfId() != shelfId) {
        qWarning() << "Shelf with name" << newName << "already exists";
        return false;
    }

    if (!library->renameShelf(shelfId, newName)) {
        qWarning() << "Shelf not found:" << shelfId;
        return false;
    }

    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::addBookToShelf(int userId, int shelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    // بررسی اینکه کاربر کتاب را دارد
    if (!library->ownsBook(bookId)) {
        qWarning() << "User does not own book:" << bookId;
        return false;
    }

    if (!library->addBookToShelf(shelfId, bookId)) {
        qWarning() << "Failed to add book to shelf";
        return false;
    }

    return m_libraryRepo->updateLibrary(library);
}

bool LibraryService::removeBookFromShelf(int userId, int shelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    if (!library->removeBookFromShelf(shelfId, bookId)) {
        qWarning() << "Failed to remove book from shelf";
        return false;
    }

    return m_libraryRepo->updateLibrary(library);
}

QVector<Shelf> LibraryService::getShelves(int userId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return QVector<Shelf>();
    }
    return library->getShelves();
}

QSharedPointer<Shelf> LibraryService::getShelfById(int userId, int shelfId) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return nullptr;
    }

    const QVector<Shelf>& shelves = library->getShelves();
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            return QSharedPointer<Shelf>::create(shelf);
        }
    }
    return nullptr;
}

QSharedPointer<Shelf> LibraryService::getShelfByName(int userId, const QString& name) const
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        return nullptr;
    }

    const QVector<Shelf>& shelves = library->getShelves();
    for (const Shelf& shelf : shelves) {
        if (shelf.getName() == name) {
            return QSharedPointer<Shelf>::create(shelf);
        }
    }
    return nullptr;
}


bool LibraryService::moveBookBetweenShelves(int userId, int fromShelfId, int toShelfId, int bookId)
{
    QSharedPointer<Library> library = getLibraryByUserId(userId);
    if (!library) {
        qWarning() << "Library not found for user:" << userId;
        return false;
    }

    // بررسی اینکه کاربر کتاب را دارد
    if (!library->ownsBook(bookId)) {
        qWarning() << "User does not own book:" << bookId;
        return false;
    }

    // حذف از قفسه مبدا
    if (!library->removeBookFromShelf(fromShelfId, bookId)) {
        qWarning() << "Failed to remove book from source shelf";
        return false;
    }

    // اضافه به قفسه مقصد
    if (!library->addBookToShelf(toShelfId, bookId)) {
        // Rollback: اگر اضافه نشد، کتاب را به قفسه مبدا برگردان
        library->addBookToShelf(fromShelfId, bookId);
        qWarning() << "Failed to add book to destination shelf, rolled back";
        return false;
    }

    return m_libraryRepo->updateLibrary(library);
}