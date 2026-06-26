// bookrepository.h
#ifndef BOOKREPOSITORY_H
#define BOOKREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Book.h"

class BookRepository {
private:
    QMap<int, Book*> booksById;     // Fast lookup by ID
    int nextId = 1000;              // Auto-increment ID for new books

public:
    BookRepository();
    ~BookRepository();

    bool addBook(Book* book);
    Book* findById(int id) const;
    QVector<Book*> getAllBooks() const;
    bool updateBook(Book* book);

    bool deleteBook(int bookId);

    int getNextId() { return nextId++; }

    // ===== Phase 3: Load/Save to file =====
    bool loadFromFile(const QString& filename);
    bool saveToFile(const QString& filename) const;
};

#endif // BOOKREPOSITORY_H