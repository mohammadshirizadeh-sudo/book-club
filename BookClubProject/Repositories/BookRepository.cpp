// bookrepository.cpp
#include "BookRepository.h"
#include <QDebug>

BookRepository::BookRepository() {
    // TODO: Load from file in Phase 3
}

BookRepository::~BookRepository() {
    // Clean up all books to prevent memory leak
    qDeleteAll(booksById);
}

bool BookRepository::addBook(Book* book) {
    if (!book) return false;

    int id = book->getBookId();

    // Check if book already exists
    if (booksById.contains(id)) {
        qWarning() << "Book with ID" << id << "already exists!";
        return false;
    }

    booksById[id] = book;
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
    return true;
}

bool BookRepository::deleteBook(int bookId) {
    Book* book = booksById.value(bookId, nullptr);
    if (!book) {
        qWarning() << "Book with ID" << bookId << "not found!";
        return false;
    }

    booksById.remove(bookId);
    delete book;
    return true;
}

bool BookRepository::loadFromFile(const QString& filename) {
    // TODO: Implement in Phase 3 (JSON or SQL)
    return false;
}

bool BookRepository::saveToFile(const QString& filename) const {
    // TODO: Implement in Phase 3 (JSON or SQL)
    return false;
}