// bookrepository.h
#ifndef BOOKREPOSITORY_H
#define BOOKREPOSITORY_H

#include <QMap>
#include <QVector>
#include "../Shared/Book.h"
#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"

class BookRepository :public QObject {
    Q_OBJECT
private:
    QMap<int, Book*> booksById;
    int nextId = 1000;


    void addToCache(Book* book);
    void removeFromCache(int bookId);
    void clearCache();

public:
    BookRepository(QObject* parent = nullptr);
    ~BookRepository();

    bool addBook(Book* book);
    Book* findById(int id) const;
    QVector<Book*> getAllBooks() const;
    bool updateBook(Book* book);

    bool deleteBook(int bookId);

    int getNextId() { return nextId++; }

    // ===== Phase 3: Load/Save to file =====

    bool loadAllFromDatabase();
    bool saveToDatabase(Book* book);
    bool deleteFromDatabase(int bookId);


};

#endif // BOOKREPOSITORY_H