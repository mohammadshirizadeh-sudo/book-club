// publisherservice.cpp
#include "PublisherService.h"

BookRepository *PublisherService::getBookRepo() const
{
    return bookRepo;
}

BookService *PublisherService::getBookService() const
{
    return bookService;
}

QMap<QString, QVariant> PublisherService::getSalesStatistics(int publisherId) const
{
    QMap<QString, QVariant> stats;

    QVector<QSharedPointer<Book>> books = bookService->getBooksByPublisher(publisherId);

    stats["totalBooks"] = books.size();
    stats["totalSales"] = getTotalSalesCount(publisherId);
    stats["totalRevenue"] = getTotalRevenue(publisherId);

    double totalRating = 0.0;
    int ratedBooks = 0;
    for (QSharedPointer<Book> book : books) {
        if (book->getAverageRating() > 0) {
            totalRating += book->getAverageRating();
            ratedBooks++;
        }
    }
    stats["averageRating"] = ratedBooks > 0 ? totalRating / ratedBooks : 0.0;


    int activeBooks = 0;
    for (QSharedPointer<Book> book : books) {
        if (book->getIsActive()) activeBooks++;
    }
    stats["activeBooks"] = activeBooks;
    stats["deactivatedBooks"] = books.size() - activeBooks;

    return stats;
}

PublisherService::PublisherService(BookService* bookService ,BookRepository* repo , UserRepository* userRepo , QObject* parent)
    : bookService(bookService) ,bookRepo(repo) , userRepo(userRepo) , QObject(parent) {
}


int PublisherService::getBooksPublishedCount(int publisherId) const {
    int count = 0;
    for (QSharedPointer<Book> book : bookRepo->getAllBooks()) {
        if (book->getPublisherId() == publisherId && book->getIsActive()) {
            count++;
        }
    }
    return count;
}

QVector<QSharedPointer<Book>> PublisherService::getBooksByPublisher(int publisherId) const {
    QVector<QSharedPointer<Book>> result;
    for (QSharedPointer<Book> book : bookRepo->getAllBooks()) {
        if (book->getPublisherId() == publisherId && book->getIsActive()) {
            result.append(book);
        }
    }
    return result;
}






bool PublisherService::addBook(int publisherId, const QString& title, const QString& author,
                               const Genre& genre, const QString& description, double price,
                               double discountPercent, const QString& coverPath,
                               const QString& pdfPath)
{
    // 1. Validate input
    if (title.isEmpty() || author.isEmpty()) {
        qWarning() << "Title and author cannot be empty";
        return false;
    }

    if (price < 0) {
        qWarning() << "Price cannot be negative";
        return false;
    }

    if (discountPercent < 0 || discountPercent > 100) {
        qWarning() << "Discount percent must be between 0 and 100";
        return false;
    }

    // 2. Check if publisher exists
    User* publisher = userRepo->findById(publisherId);
    if (!publisher) {
        qWarning() << "Publisher not found with ID:" << publisherId;
        return false;
    }

    if (!publisher->isPublisher()) {
        qWarning() << "User" << publisherId << "is not a publisher";
        return false;
    }

    // 3. Create book
    QSharedPointer<Book> book = QSharedPointer<Book>::create(0, title, author, genre, price, publisherId);
    // Book* book = new Book(0, title, author, genre, price, publisherId);
    book->setDescription(description);
    book->setDiscountPercent(discountPercent);
    book->setCoverPath(coverPath);
    book->setPdfPath(pdfPath);
    book->activate();
    book->setCreatedAt(QDateTime::currentDateTime());
    book->setUpdatedAt(QDateTime::currentDateTime());

    int newId = bookService->addBook(book);

    if (newId == -1) {
        qWarning() << "Failed to add book:" << title;
        return false;
    }


    qDebug() << "✅ Book added by publisher" << publisher->getUsername()
             << ":" << title << "(" << GenreHelper::toString(genre) << ")";
    return true;
}

bool PublisherService::editBook(int bookId, const QString& newTitle,
                                const QString& newAuthor, const Genre& newGenre,
                                const QString& newDescription, double newPrice,
                                double newDiscountPercent)
{
    return bookService->editBook(bookId , newTitle , newAuthor , newGenre , newDescription , newPrice , newDiscountPercent);
}


double PublisherService::getTotalRevenue(int publisherId) const
{
    double total = 0.0;
    for (QSharedPointer<Book> book : bookService->getBooksByPublisher(publisherId)) {
        total += book->getFinalPrice() * book->getSalesCount();
    }
    return total;
}

int PublisherService::getTotalSalesCount(int publisherId) const
{
    int total = 0;
    for (QSharedPointer<Book> book : bookService->getBooksByPublisher(publisherId)) {
        total += book->getSalesCount();
    }
    return total;
}