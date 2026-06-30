// shelf.cpp
#include "Shelf.h"
#include <QDebug>

// ===== Constructors =====
Shelf::Shelf()
    : shelfId(0)
    , name("") {
}

Shelf::Shelf(int shelfId, const QString& name)
    : shelfId(shelfId)
    , name(name) {
}

// ===== Book Management =====

bool Shelf::addBook(int bookId) {
    if (bookId <= 0) {
        qWarning() << "Invalid book ID:" << bookId;
        return false;
    }

    if (contains(bookId)) {
        qWarning() << "Book" << bookId << "already exists in shelf";
        return false;
    }

    bookIds.append(bookId);
    return true;
}

bool Shelf::removeBook(int bookId) {
    return bookIds.removeOne(bookId);
}

bool Shelf::contains(int bookId) const {
    return bookIds.contains(bookId);
}

void Shelf::clear() {
    bookIds.clear();
}