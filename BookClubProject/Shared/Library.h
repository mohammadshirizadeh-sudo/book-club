// library.h
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QString>
#include <QVector>
#include "Shelf.h"

/**
 * @brief Library class representing user's personal library
 */
class Library {
private:
    int userId;
    QVector<int> ownedBooks;     // Books purchased by user
    QVector<int> savedBooks;     // Books saved for later
    QVector<Shelf> shelves;      // Custom shelves created by user

public:
    // ===== Constructors =====
    Library();
    explicit Library(int userId);

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

    /**
     * @brief Add a book to owned books (after purchase)
     * @param bookId ID of book to add
     * @return true if added successfully
     */
    bool addOwnedBook(int bookId);

    /**
     * @brief Remove a book from owned books
     * @param bookId ID of book to remove
     * @return true if removed successfully
     */
    bool removeOwnedBook(int bookId);

    /**
     * @brief Check if user owns a book
     * @param bookId ID of book
     * @return true if user owns the book
     */
    bool ownsBook(int bookId) const;

    // ===== Saved Books Management =====

    /**
     * @brief Save a book for later
     * @param bookId ID of book to save
     * @return true if saved successfully
     */
    bool saveBook(int bookId);

    /**
     * @brief Unsave a book
     * @param bookId ID of book to unsave
     * @return true if unsaved successfully
     */
    bool unsaveBook(int bookId);

    /**
     * @brief Check if book is saved
     * @param bookId ID of book
     * @return true if book is saved
     */
    bool isBookSaved(int bookId) const;

    // ===== Shelf Management =====

    /**
     * @brief Create a new shelf
     * @param name Shelf name
     * @return true if created successfully
     */
    bool createShelf(const QString& name);

    /**
     * @brief Create a new shelf with specific ID
     * @param shelfId ID for the shelf
     * @param name Shelf name
     * @return true if created successfully
     */
    bool createShelf(int shelfId, const QString& name);

    /**
     * @brief Delete a shelf
     * @param shelfId ID of shelf to delete
     * @return true if deleted successfully
     */
    bool deleteShelf(int shelfId);

    /**
     * @brief Rename a shelf
     * @param shelfId ID of shelf
     * @param newName New shelf name
     * @return true if renamed successfully
     */
    bool renameShelf(int shelfId, const QString& newName);

    /**
     * @brief Get shelf by ID
     * @param shelfId ID of shelf
     * @return Shelf pointer or nullptr if not found
     */
    Shelf* getShelf(int shelfId);
    const Shelf* getShelf(int shelfId) const;

    /**
     * @brief Find shelf by name
     * @param name Shelf name
     * @return Shelf pointer or nullptr if not found
     */
    Shelf* findShelfByName(const QString& name);
    const Shelf* findShelfByName(const QString& name) const;

    /**
     * @brief Add book to a specific shelf
     * @param shelfId ID of shelf
     * @param bookId ID of book
     * @return true if added successfully
     */
    bool addBookToShelf(int shelfId, int bookId);

    /**
     * @brief Remove book from a specific shelf
     * @param shelfId ID of shelf
     * @param bookId ID of book
     * @return true if removed successfully
     */
    bool removeBookFromShelf(int shelfId, int bookId);

    /**
     * @brief Move book from one shelf to another
     * @param fromShelfId Source shelf ID
     * @param toShelfId Destination shelf ID
     * @param bookId ID of book
     * @return true if moved successfully
     */
    bool moveBookBetweenShelves(int fromShelfId, int toShelfId, int bookId);

    /**
     * @brief Get all books in a shelf
     * @param shelfId ID of shelf
     * @return Vector of book IDs
     */
    QVector<int> getBooksInShelf(int shelfId) const;

    /**
     * @brief Get all books that are in any shelf
     * @return Vector of book IDs
     */
    QVector<int> getAllBooksInShelves() const;

    /**
     * @brief Get all books not in any shelf
     * @return Vector of book IDs
     */
    QVector<int> getUnshelvedBooks() const;
};

#endif // LIBRARY_H