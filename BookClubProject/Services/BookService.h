// bookservice.h
#ifndef BOOKSERVICE_H
#define BOOKSERVICE_H

#include <QString>
#include <QVector>
#include "../Shared/Book.h"
#include "../Repositories/BookRepository.h"
#include "../Repositories/ReviewRepository.h"
#include "../Shared/Genre.h"

class BookService : public QObject {
    Q_OBJECT
private:
    BookRepository* bookRepo;
    ReviewRepository* reviewRepo;


    bool areGenresRelated(Genre genre1, Genre genre2) const;

public:
    // ===== Constructor =====
    explicit BookService(BookRepository* repo, ReviewRepository* reviewRepo = nullptr , QObject* parent = nullptr);

    // ===== Book Management =====


    int addBook(QSharedPointer<Book> book);

    bool editBook(int bookId, const QString& newTitle,
                  const QString& newAuthor, const Genre& newGenre,
                  const QString& newDescription, double newPrice,
                  double newDiscountPercent);


    bool deactivateBook(int bookId);

    bool reactivateBook(int bookId);

    // ===== Search & Filter =====

    QVector<QSharedPointer<Book>> searchBooks(const QString& keyword) const;

    QVector<QSharedPointer<Book>> filterByGenre(const QString& genre) const;

    QVector<QSharedPointer<Book>> filterByPrice(double minPrice, double maxPrice) const;

    QVector<QSharedPointer<Book>> filterByAuthor(const QString& author) const;

    QVector<QSharedPointer<Book>> filterByPublisher(int publisherId) const;

    // ===== Recommendations =====

    QVector<QSharedPointer<Book>> getRecommendedBooks(const QVector<Genre>& favoriteGenres, int limit = 10) const;

    QVector<QSharedPointer<Book>> getPopularBooks(int limit = 10) const;

    QVector<QSharedPointer<Book>> getNewBooks(int limit = 10) const;

    QVector<QSharedPointer<Book>> getFreeBooks() const;

    QVector<QSharedPointer<Book>> getBooksByGenre(const QString& genre) const;

    // ===== Helper Methods =====
    QSharedPointer<Book> getBookById(int bookId) const;
    QVector<QSharedPointer<Book>> getAllBooks() const;

    QVector<QSharedPointer<Book>> getBooksByPublisher(int publisherId) const;

    bool updateAverageRating(int bookId, double newRating);

    bool updateSalesCount(int bookId, int quantity);
    BookRepository *getBookRepo() const;


    bool deleteBook(int bookId);

    int calculateGenreMatchScore(const QSharedPointer<Book> book, const QVector<Genre>& favoriteGenres) const;


     QMap<QString, QVector<QSharedPointer<Book>>> searchAuthorsWithBooks(const QString& keyword) const;
};

#endif // BOOKSERVICE_H