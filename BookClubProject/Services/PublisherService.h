// publisherservice.h
#ifndef PUBLISHERSERVICE_H
#define PUBLISHERSERVICE_H

#include <QVector>
#include "../Shared/Publisher.h"
#include "../Repositories/BookRepository.h"
#include "../Services/BookService.h"
#include "../Repositories/UserRepository.h"

class PublisherService : public QObject {
    Q_OBJECT
private:
    BookRepository* bookRepo;
    BookService* bookService;

    UserRepository* userRepo;




    double getTotalRevenue(int publisherId) const;
    int getTotalSalesCount(int publisherId) const;

public:
    explicit PublisherService(BookService* bookService ,BookRepository* repo , UserRepository* userRepo  , QObject* parent = nullptr);

    int getBooksPublishedCount(int publisherId) const;
    QVector<Book*> getBooksByPublisher(int publisherId) const;
    BookRepository *getBookRepo() const;
    BookService *getBookService() const;
    QMap<QString, QVariant> getSalesStatistics(int publisherId) const;
    bool addBook(int publisherId, const QString& title, const QString& author,
                 const Genre& genre, const QString& description, double price,
                 double discountPercent = 0.0,
                 const QString& coverPath = "", const QString& pdfPath = "");
    bool editBook(int bookId, const QString& newTitle,
                  const QString& newAuthor, const Genre& newGenre,
                  const QString& newDescription, double newPrice,
                  double newDiscountPercent);

};


#endif // PUBLISHERSERVICE_H