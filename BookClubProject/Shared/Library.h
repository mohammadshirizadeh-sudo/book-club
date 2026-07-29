// library.h
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QString>
#include <QVector>
#include "Shelf.h"
#include <QDateTime>

class Library {
private:
    int userId;
    QVector<int> ownedBooks;
    QVector<int> savedBooks;
    QVector<Shelf> shelves;
    QDateTime createdAt;
    QDateTime updatedAt;
    \
public:
    Library();
    explicit Library(int userId);
    Library(int userId, const QVector<int>& ownedBooks, const QVector<int>& savedBooks,
            const QVector<Shelf>& shelves ,QDateTime createdAt,QDateTime updatedAt );

    // ===== Getters =====
    int getUserId() const { return userId; }
    QVector<int> getOwnedBooks() const { return ownedBooks; }
    QVector<int> getSavedBooks() const { return savedBooks; }
    QVector<Shelf> getShelves() const { return shelves; }
    int getOwnedBookCount() const { return ownedBooks.size(); }
    int getSavedBookCount() const { return savedBooks.size(); }
    int getShelfCount() const { return shelves.size(); }

    // ===== Setters =====
    void setUserId(int id) { userId = id; }
    void setOwnedBooks(const QVector<int>& books) { ownedBooks = books; }
    void setSavedBooks(const QVector<int>& books) { savedBooks = books; }
    void setShelves(const QVector<Shelf>& shelves) { this->shelves = shelves; }

    // ===== Owned Books Management =====
    bool addOwnedBook(int bookId);
    bool removeOwnedBook(int bookId);
    bool ownsBook(int bookId) const;

    bool saveBook(int bookId);
    bool unsaveBook(int bookId);
    bool isBookSaved(int bookId) const;

    bool createShelf(const QString& name);
    bool createShelf(int shelfId, const QString& name);
    bool deleteShelf(int shelfId);
    bool renameShelf(int shelfId, const QString& newName);
    Shelf* getShelf(int shelfId);
    const Shelf* getShelf(int shelfId) const;
    Shelf* findShelfByName(const QString& name);
    const Shelf* findShelfByName(const QString& name) const;
    bool addBookToShelf(int shelfId, int bookId);
    bool removeBookFromShelf(int shelfId, int bookId);
    bool moveBookBetweenShelves(int fromShelfId, int toShelfId, int bookId);
    QVector<int> getBooksInShelf(int shelfId) const;
    QVector<int> getAllBooksInShelves() const;
    QVector<int> getUnshelvedBooks() const;


};

#endif // LIBRARY_H