// bookservice.h
#ifndef BOOKSERVICE_H
#define BOOKSERVICE_H

#include <QString>
#include <QVector>
#include "../Shared/Book.h"
#include "../Repositories/BookRepository.h"
#include "../Repositories/ReviewRepository.h"

/**
 * @brief Book Service - Handles book management operations
 */
class BookService {
private:
    BookRepository* bookRepo;
    ReviewRepository* reviewRepo;

public:
    // ===== Constructor =====
    explicit BookService(BookRepository* repo, ReviewRepository* reviewRepo = nullptr);

    // ===== Book Management =====


    bool addBook(Book* book);

    bool editBook(int bookId, const QString& newTitle,
                  const QString& newAuthor, const QString& newGenre,
                  const QString& newDescription, double newPrice,
                  double newDiscountPercent);


    bool deactivateBook(int bookId);

    bool reactivateBook(int bookId);

    // ===== Search & Filter =====

    QVector<Book*> searchBooks(const QString& keyword) const;

    QVector<Book*> filterByGenre(const QString& genre) const;

    QVector<Book*> filterByPrice(double minPrice, double maxPrice) const;

    QVector<Book*> filterByAuthor(const QString& author) const;

    QVector<Book*> filterByPublisher(int publisherId) const;

    // ===== Recommendations =====

    QVector<Book*> getRecommendedBooks(const QVector<QString>& favoriteGenres, int limit = 10) const;

    QVector<Book*> getPopularBooks(int limit = 10) const;

    QVector<Book*> getNewBooks(int limit = 10) const;

    QVector<Book*> getFreeBooks() const;

    QVector<Book*> getBooksByGenre(const QString& genre) const;

    // ===== Helper Methods =====
    Book* getBookById(int bookId) const;
    QVector<Book*> getAllBooks() const;

    QVector<Book*> getBooksByPublisher(int publisherId) const;

    bool updateAverageRating(int bookId, double newRating);

    bool updateSalesCount(int bookId, int quantity);
    BookRepository *getBookRepo() const;
};

#endif // BOOKSERVICE_H