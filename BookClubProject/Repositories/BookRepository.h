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
    // QMap<int, Book*> booksById;
    QMap<int, QSharedPointer<Book>> booksById;
    QAtomicInteger<int> nextId{1000};

    mutable QMutex m_mutex;


    void addToCache(QSharedPointer<Book> book);
    void removeFromCache(int bookId);
    void clearCache();

public:
    BookRepository(QObject* parent = nullptr);
    ~BookRepository();

    bool addBook(QSharedPointer<Book> book);
    // Book* findById(int id) const;
    QSharedPointer<Book> findById(int id)const;
    // QVector<Book*> getAllBooks() const;
    QVector<QSharedPointer<Book>> getAllBooks()const;
    bool updateBook(QSharedPointer<Book> book);

    bool deleteBook(int bookId);

    int getNextId() { return nextId++; }

    // ===== Phase 3: Load/Save to file =====

    bool loadAllFromDatabase();
    bool saveToDatabase(QSharedPointer<Book> book);
    bool deleteFromDatabase(int bookId);


};

#endif // BOOKREPOSITORY_H