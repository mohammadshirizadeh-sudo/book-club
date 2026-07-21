// LibraryService.h
#ifndef LIBRARYSERVICE_H
#define LIBRARYSERVICE_H

#include <QObject>
#include <QSharedPointer>
#include "../Repositories/LibraryRepository.h"
#include "../Shared/Library.h"
#include "../Shared/Shelf.h"

class LibraryService : public QObject
{
    Q_OBJECT

public:
    explicit LibraryService(LibraryRepository* libraryRepo, QObject* parent = nullptr);

    // ===== Library Management =====
    QSharedPointer<Library> getLibraryByUserId(int userId) const;
    bool createLibrary(int userId);
    bool deleteLibrary(int userId);
    bool libraryExists(int userId) const;

    // ===== Owned Books =====
    bool addOwnedBook(int userId, int bookId);
    bool removeOwnedBook(int userId, int bookId);
    bool ownsBook(int userId, int bookId) const;
    QVector<int> getOwnedBooks(int userId) const;

    // ===== Saved Books =====
    bool saveBook(int userId, int bookId);
    bool unsaveBook(int userId, int bookId);
    bool isBookSaved(int userId, int bookId) const;
    QVector<int> getSavedBooks(int userId) const;

    // ===== Shelf Management =====
    bool createShelf(int userId, const QString& shelfName);
    bool deleteShelf(int userId, int shelfId);
    bool renameShelf(int userId, int shelfId, const QString& newName);
    bool addBookToShelf(int userId, int shelfId, int bookId);
    bool removeBookFromShelf(int userId, int shelfId, int bookId);
    QVector<Shelf> getShelves(int userId) const;
    QSharedPointer<Shelf> getShelfById(int userId, int shelfId) const;
    QSharedPointer<Shelf> getShelfByName(int userId, const QString& name) const;
    bool moveBookBetweenShelves(int userId, int fromShelfId, int toShelfId, int bookId);




private:
    LibraryRepository* m_libraryRepo;
};

#endif // LIBRARYSERVICE_H