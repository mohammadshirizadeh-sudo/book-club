// library.cpp
#include "Library.h"
#include <QDebug>

// ===== Constructors =====
Library::Library()
    : userId(0) {
}

Library::Library(int userId)
    : userId(userId) {
}

Library::Library(int userId, const QVector<int>& ownedBooks, const QVector<int>& savedBooks,
                 const QVector<Shelf>& shelves)
    : userId(userId)
    , ownedBooks(ownedBooks)
    , savedBooks(savedBooks)
    , shelves(shelves)

{
}

// ===== Owned Books Management =====

bool Library::addOwnedBook(int bookId) {
    if (bookId <= 0) {
        qWarning() << "Invalid book ID:" << bookId;
        return false;
    }

    if (ownsBook(bookId)) {
        qWarning() << "Book" << bookId << "already owned";
        return false;
    }

    ownedBooks.append(bookId);
    return true;
}

bool Library::removeOwnedBook(int bookId) {
    return ownedBooks.removeOne(bookId);
}

bool Library::ownsBook(int bookId) const {
    return ownedBooks.contains(bookId);
}

// ===== Saved Books Management =====

bool Library::saveBook(int bookId) {
    if (bookId <= 0) {
        qWarning() << "Invalid book ID:" << bookId;
        return false;
    }

    if (isBookSaved(bookId)) {
        qWarning() << "Book" << bookId << "already saved";
        return false;
    }

    savedBooks.append(bookId);
    return true;
}

bool Library::unsaveBook(int bookId) {
    return savedBooks.removeOne(bookId);
}

bool Library::isBookSaved(int bookId) const {
    return savedBooks.contains(bookId);
}

// ===== Shelf Management =====

bool Library::createShelf(const QString& name) {
    if (name.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    // Check if shelf name already exists
    if (findShelfByName(name) != nullptr) {
        qWarning() << "Shelf with name" << name << "already exists";
        return false;
    }

    // Generate new shelf ID
    int maxId = 0;
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() > maxId) {
            maxId = shelf.getShelfId();
        }
    }

    Shelf newShelf(maxId + 1, name);
    shelves.append(newShelf);
    return true;
}

bool Library::createShelf(int shelfId, const QString& name) {
    if (name.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    // Check if shelf ID already exists
    if (getShelf(shelfId) != nullptr) {
        qWarning() << "Shelf with ID" << shelfId << "already exists";
        return false;
    }

    // Check if shelf name already exists
    if (findShelfByName(name) != nullptr) {
        qWarning() << "Shelf with name" << name << "already exists";
        return false;
    }

    Shelf newShelf(shelfId, name);
    shelves.append(newShelf);
    return true;
}

bool Library::deleteShelf(int shelfId) {
    for (int i = 0; i < shelves.size(); ++i) {
        if (shelves[i].getShelfId() == shelfId) {
            shelves.remove(i);
            return true;
        }
    }
    return false;
}

bool Library::renameShelf(int shelfId, const QString& newName) {
    if (newName.isEmpty()) {
        qWarning() << "Shelf name cannot be empty";
        return false;
    }

    Shelf* shelf = getShelf(shelfId);
    if (!shelf) {
        qWarning() << "Shelf" << shelfId << "not found";
        return false;
    }

    // Check if name is already used by another shelf
    const Shelf* existing = findShelfByName(newName);
    if (existing && existing->getShelfId() != shelfId) {
        qWarning() << "Shelf name" << newName << "already used";
        return false;
    }

    shelf->setName(newName);
    return true;
}

Shelf* Library::getShelf(int shelfId) {
    for (Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            return &shelf;
        }
    }
    return nullptr;
}

const Shelf* Library::getShelf(int shelfId) const {
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            return &shelf;
        }
    }
    return nullptr;
}

Shelf* Library::findShelfByName(const QString& name) {
    for (Shelf& shelf : shelves) {
        if (shelf.getName() == name) {
            return &shelf;
        }
    }
    return nullptr;
}

const Shelf* Library::findShelfByName(const QString& name) const {
    for (const Shelf& shelf : shelves) {
        if (shelf.getName() == name) {
            return &shelf;
        }
    }
    return nullptr;
}

bool Library::addBookToShelf(int shelfId, int bookId) {
    Shelf* shelf = getShelf(shelfId);
    if (!shelf) {
        qWarning() << "Shelf" << shelfId << "not found";
        return false;
    }

    // Check if book is owned by user
    if (!ownsBook(bookId)) {
        qWarning() << "User does not own book" << bookId;
        return false;
    }

    return shelf->addBook(bookId);
}

bool Library::removeBookFromShelf(int shelfId, int bookId) {
    Shelf* shelf = getShelf(shelfId);
    if (!shelf) {
        qWarning() << "Shelf" << shelfId << "not found";
        return false;
    }

    return shelf->removeBook(bookId);
}

bool Library::moveBookBetweenShelves(int fromShelfId, int toShelfId, int bookId) {
    if (fromShelfId == toShelfId) {
        qWarning() << "Source and destination shelves are the same";
        return false;
    }

    if (!removeBookFromShelf(fromShelfId, bookId)) {
        return false;
    }

    if (!addBookToShelf(toShelfId, bookId)) {
        // Rollback: add back to source shelf
        addBookToShelf(fromShelfId, bookId);
        return false;
    }

    return true;
}

QVector<int> Library::getBooksInShelf(int shelfId) const {
    const Shelf* shelf = getShelf(shelfId);
    if (!shelf) {
        return QVector<int>();
    }
    return shelf->getBookIds();
}

QVector<int> Library::getAllBooksInShelves() const {
    QVector<int> allBooks;
    for (const Shelf& shelf : shelves) {
        QVector<int> bookIds = shelf.getBookIds();
        for (int bookId : bookIds) {
            if (!allBooks.contains(bookId)) {
                allBooks.append(bookId);
            }
        }
    }
    return allBooks;
}

QVector<int> Library::getUnshelvedBooks() const {
    QVector<int> unshelved;
    QVector<int> shelvedBooks = getAllBooksInShelves();

    for (int bookId : ownedBooks) {
        if (!shelvedBooks.contains(bookId)) {
            unshelved.append(bookId);
        }
    }

    return unshelved;
}