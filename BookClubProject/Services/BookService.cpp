// bookservice.cpp
#include "BookService.h"

#include <QDebug>
#include <algorithm>

// ===== Constructor =====
BookRepository *BookService::getBookRepo() const
{
    return bookRepo;
}

bool BookService::deleteBook(int bookId)
{
    // 1. Check if book exists
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // 2. Log the deletion
    qDebug() << "Deleting book:" << book->getTitle() << "(ID:" << bookId << ")";

    // 3. Delete from repository
    if (!bookRepo->deleteBook(bookId)) {
        qWarning() << "Failed to delete book:" << bookId;
        return false;
    }

    qDebug() << "Book deleted successfully:" << book->getTitle();
    return true;
}


BookService::BookService(BookRepository* repo, ReviewRepository* reviewRepo , QObject* parent)

    : bookRepo(repo), reviewRepo(reviewRepo) ,QObject(parent) {

}

// ===== Book Management =====

bool BookService::addBook(Book* book) {
    if (!book) {
        qWarning() << "Book is null!";
        return false;
    }

    // Validate book data
    if (book->getTitle().isEmpty()) {
        qWarning() << "Book title cannot be empty!";
        return false;
    }

    if (book->getAuthor().isEmpty()) {
        qWarning() << "Book author cannot be empty!";
        return false;
    }

    if (book->getPrice() < 0) {
        qWarning() << "Book price cannot be negative!";
        return false;
    }

    // Add to repository
    if (!bookRepo->addBook(book)) {
        qWarning() << "Failed to add book to repository!";
        return false;
    }

    qDebug() << "Book added:" << book->getTitle() << "by" << book->getAuthor();
    return true;
}

bool BookService::editBook(int bookId, const QString& newTitle,
                           const QString& newAuthor, const Genre& newGenre,
                           const QString& newDescription, double newPrice,
                           double newDiscountPercent) {
    // 1. Find book
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // 2. Validate new data
    if (newTitle.isEmpty()) {
        qWarning() << "Book title cannot be empty!";
        return false;
    }

    if (newAuthor.isEmpty()) {
        qWarning() << "Book author cannot be empty!";
        return false;
    }

    if (newPrice < 0) {
        qWarning() << "Book price cannot be negative!";
        return false;
    }

    if (newDiscountPercent < 0 || newDiscountPercent > 100) {
        qWarning() << "Invalid discount percentage:" << newDiscountPercent;
        return false;
    }

    // 3. Update book data
    book->setTitle(newTitle);
    book->setAuthor(newAuthor);
    book->setGenre(newGenre);
    book->setDescription(newDescription);
    book->setPrice(newPrice);
    book->setDiscountPercent(newDiscountPercent);
    book->setUpdatedAt(QDateTime::currentDateTime());

    // 4. Save to repository
    if (!bookRepo->updateBook(book)) {
        qWarning() << "Failed to update book in repository!";
        return false;
    }

    qDebug() << "Book updated:" << book->getTitle();
    return true;
}

bool BookService::deactivateBook(int bookId) {
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    if (!book->getIsActive()) {
        qWarning() << "Book is already deactivated!";
        return false;
    }

    book->deactivate();
    bookRepo->updateBook(book);

    qDebug() << "Book deactivated:" << book->getTitle();
    return true;
}

bool BookService::reactivateBook(int bookId) {
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    if (book->getIsActive()) {
        qWarning() << "Book is already active!";
        return false;
    }

    book->activate();
    bookRepo->updateBook(book);

    qDebug() << "Book reactivated:" << book->getTitle();
    return true;
}

// ===== Search & Filter =====

QVector<Book*> BookService::searchBooks(const QString& keyword) const {
    if (keyword.isEmpty()) {
        return QVector<Book*>();
    }

    QVector<Book*> results;
    QString lowerKeyword = keyword.toLower();

    for (Book* book : bookRepo->getAllBooks()) {
        // Only search active books
        if (!book->getIsActive()) continue;


        QString genreOf = GenreHelper::toString(book->getGenre());

        // Search by title, author, genre, or description
        if (book->getTitle().toLower().contains(lowerKeyword) ||
            book->getAuthor().toLower().contains(lowerKeyword) ||
            genreOf.toLower().contains(lowerKeyword) ||
            book->getDescription().toLower().contains(lowerKeyword)) {
            results.append(book);
        }
    }

    return results;
}

QVector<Book*> BookService::filterByGenre(const QString& genre) const {
    QVector<Book*> results;

    for (Book* book : bookRepo->getAllBooks()) {
        if (!book->getIsActive()) continue;


        QString genreOf = GenreHelper::toString(book->getGenre());

        if (genreOf.toLower() == genre.toLower()) {
            results.append(book);
        }
    }

    return results;
}

QVector<Book*> BookService::filterByPrice(double minPrice, double maxPrice) const {
    QVector<Book*> results;

    for (Book* book : bookRepo->getAllBooks()) {
        if (!book->getIsActive()) continue;

        double finalPrice = book->getFinalPrice();
        if (finalPrice >= minPrice && finalPrice <= maxPrice) {
            results.append(book);
        }
    }

    return results;
}

QVector<Book*> BookService::filterByAuthor(const QString& author) const {
    QVector<Book*> results;

    for (Book* book : bookRepo->getAllBooks()) {
        if (!book->getIsActive()) continue;

        if (book->getAuthor().toLower().contains(author.toLower())) {
            results.append(book);
        }
    }

    return results;
}

QVector<Book*> BookService::filterByPublisher(int publisherId) const {
    QVector<Book*> results;

    for (Book* book : bookRepo->getAllBooks()) {
        if (!book->getIsActive()) continue;

        if (book->getPublisherId() == publisherId) {
            results.append(book);
        }
    }

    return results;
}

// ===== Recommendations =====

/*QVector<Book*> BookService::getRecommendedBooks(const QVector<QString>& favoriteGenres, int limit) const {
    if (favoriteGenres.isEmpty()) {
        qWarning() << "No favorite genres provided!";
        return QVector<Book*>();
    }

    QVector<Book*> recommendations;

    // Get all active books
    QVector<Book*> allBooks = bookRepo->getAllBooks();

    // Score each book based on genre match
    QVector<QPair<Book*, int>> scoredBooks;

    for (Book* book : allBooks) {
        if (!book->getIsActive()) continue;

        int score = 0;
        QString bookGenre = book->getGenre().toLower();

        for (const QString& genre : favoriteGenres) {
            if (bookGenre == genre.toLower()) {
                score += 10;  // Exact genre match
            } else if (bookGenre.contains(genre.toLower()) ||
                       genre.toLower().contains(bookGenre)) {
                score += 5;   // Partial match
            }
        }

        // Bonus for high rating
        if (book->getAverageRating() >= 4.5) score += 3;
        else if (book->getAverageRating() >= 4.0) score += 2;

        // Bonus for popularity
        if (book->getSalesCount() > 100) score += 2;

        // Bonus for discounts
        if (book->isDiscounted()) score += 1;

        if (score > 0) {
            scoredBooks.append(qMakePair(book, score));
        }
    }

    // Sort by score (descending)
    std::sort(scoredBooks.begin(), scoredBooks.end(),
              [](const QPair<Book*, int>& a, const QPair<Book*, int>& b) {
                  return a.second > b.second;
              });

    // Take top 'limit' books
    int count = qMin(limit, scoredBooks.size());
    for (int i = 0; i < count; ++i) {
        recommendations.append(scoredBooks[i].first);
    }

    return recommendations;
}*/

QVector<Book*> BookService::getRecommendedBooks(const QVector<Genre>& favoriteGenres, int limit) const {
    if (favoriteGenres.isEmpty()) {
        qWarning() << "No favorite genres provided!";
        return QVector<Book*>();
    }

    QVector<Book*> recommendations;

    // Get all active books
    QVector<Book*> allBooks = bookRepo->getAllBooks();

    // Score each book based on genre match
    QVector<QPair<Book*, int>> scoredBooks;

    for (Book* book : allBooks) {
        if (!book->getIsActive()) continue;

        int score = calculateGenreMatchScore(book, favoriteGenres);

        // Bonus for high rating
        if (book->getAverageRating() >= 4.5) score += 3;
        else if (book->getAverageRating() >= 4.0) score += 2;

        // Bonus for popularity
        if (book->getSalesCount() > 100) score += 2;

        // Bonus for discounts
        if (book->getDiscountPercent() > 0) score += 1;

        if (score > 0) {
            scoredBooks.append(qMakePair(book, score));
        }
    }

    // Sort by score (descending)
    std::sort(scoredBooks.begin(), scoredBooks.end(),
              [](const QPair<Book*, int>& a, const QPair<Book*, int>& b) {
                  return a.second > b.second;
              });

    // Take top 'limit' books
    int count = qMin(limit, scoredBooks.size());
    for (int i = 0; i < count; ++i) {
        recommendations.append(scoredBooks[i].first);
    }

    return recommendations;
}


int BookService::calculateGenreMatchScore(const Book* book, const QVector<Genre>& favoriteGenres) const {
    int score = 0;
    Genre bookGenre = book->getGenre();

    for (Genre favoriteGenre : favoriteGenres) {
        if (bookGenre == favoriteGenre) {
            score += 10;  // Exact genre match
            break;
        }
    }

    if (score == 0) {
        for (Genre favoriteGenre : favoriteGenres) {
            if (areGenresRelated(bookGenre, favoriteGenre)) {
                score += 5;  // Partial/Related match
                break;
            }
        }
    }

    return score;
}


QVector<Book*> BookService::getPopularBooks(int limit) const {
    QVector<Book*> popular;
    QVector<Book*> allBooks = bookRepo->getAllBooks();

    // Sort by sales count (descending)
    std::sort(allBooks.begin(), allBooks.end(),
              [](const Book* a, const Book* b) {
                  return a->getSalesCount() > b->getSalesCount();
              });

    int count = qMin(limit, allBooks.size());
    for (int i = 0; i < count; ++i) {
        if (allBooks[i]->getIsActive()) {
            popular.append(allBooks[i]);
        }
    }

    return popular;
}

QVector<Book*> BookService::getNewBooks(int limit) const {
    QVector<Book*> newBooks;
    QVector<Book*> allBooks = bookRepo->getAllBooks();

    // Sort by createdAt (descending - newest first)
    std::sort(allBooks.begin(), allBooks.end(),
              [](const Book* a, const Book* b) {
                  return a->getCreatedAt() > b->getCreatedAt();
              });

    int count = qMin(limit, allBooks.size());
    for (int i = 0; i < count; ++i) {
        if (allBooks[i]->getIsActive()) {
            newBooks.append(allBooks[i]);
        }
    }

    return newBooks;
}

QVector<Book*> BookService::getFreeBooks() const {
    QVector<Book*> freeBooks;

    for (Book* book : bookRepo->getAllBooks()) {
        if (book->getIsActive() && book->isFree()) {
            freeBooks.append(book);
        }
    }

    return freeBooks;
}

QVector<Book*> BookService::getBooksByGenre(const QString& genre) const {
    return filterByGenre(genre);
}

// ===== Helper Methods =====

Book* BookService::getBookById(int bookId) const {
    return bookRepo->findById(bookId);
}

QVector<Book*> BookService::getAllBooks() const {
    QVector<Book*> activeBooks;
    for (Book* book : bookRepo->getAllBooks()) {
        if (book->getIsActive()) {
            activeBooks.append(book);
        }
    }
    return activeBooks;
}

QVector<Book*> BookService::getBooksByPublisher(int publisherId) const {
    return filterByPublisher(publisherId);
}

bool BookService::updateAverageRating(int bookId, double newRating) {
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    // Get all reviews for this book and calculate average
    // For now, just update with new rating (placeholder)
    // In real implementation, you would fetch all reviews and calculate

    int reviewCount = reviewRepo->getReviewsByBookId(bookId).size();

    if (reviewCount == 0) {
        // اگر اولین نظر است
        book->setAverageRating(newRating);
    } else {
        // محاسبه میانگین واقعی
        double currentAvg = book->getAverageRating();
        double newAvg = (currentAvg * reviewCount + newRating) / (reviewCount + 1);
        book->setAverageRating(newAvg);
    }


    bookRepo->updateBook(book);

    return true;
}

bool BookService::updateSalesCount(int bookId, int quantity) {
    Book* book = bookRepo->findById(bookId);
    if (!book) {
        qWarning() << "Book not found with ID:" << bookId;
        return false;
    }

    int newCount = book->getSalesCount() + quantity;
    book->setSalesCount(newCount);
    bookRepo->updateBook(book);

    return true;
}



bool BookService::areGenresRelated(Genre genre1, Genre genre2) const
{
    if (genre1 == genre2)
        return true;

    switch (genre1)
    {
    // ===== داستانی =====
    case Genre::Fiction:
        return genre2 == Genre::Fantasy ||
               genre2 == Genre::Science_Fiction ||
               genre2 == Genre::Mystery_and_Crime ||
               genre2 == Genre::Romance ||
               genre2 == Genre::Thriller ||
               genre2 == Genre::Adventure ||
               genre2 == Genre::Drama ||
               genre2 == Genre::Horror;

    case Genre::Fantasy:
        return genre2 == Genre::Fiction ||
               genre2 == Genre::Adventure ||
               genre2 == Genre::Science_Fiction;

    case Genre::Science_Fiction:
        return genre2 == Genre::Science ||
               genre2 == Genre::Technology ||
               genre2 == Genre::Fantasy ||
               genre2 == Genre::Fiction;

    case Genre::Adventure:
        return genre2 == Genre::Fantasy ||
               genre2 == Genre::Fiction ||
               genre2 == Genre::Thriller;

    case Genre::Mystery_and_Crime:
        return genre2 == Genre::Thriller ||
               genre2 == Genre::Horror ||
               genre2 == Genre::Fiction;

    case Genre::Thriller:
        return genre2 == Genre::Mystery_and_Crime ||
               genre2 == Genre::Horror ||
               genre2 == Genre::Adventure;

    case Genre::Horror:
        return genre2 == Genre::Thriller ||
               genre2 == Genre::Mystery_and_Crime ||
               genre2 == Genre::Fantasy;

    case Genre::Romance:
        return genre2 == Genre::Drama ||
               genre2 == Genre::Comedy ||
               genre2 == Genre::Fiction;

    case Genre::Drama:
        return genre2 == Genre::Romance ||
               genre2 == Genre::Poetry ||
               genre2 == Genre::Fiction;

    case Genre::Comedy:
        return genre2 == Genre::Romance ||
               genre2 == Genre::Drama;

    case Genre::Comics:
        return genre2 == Genre::Manga ||
               genre2 == Genre::Fantasy ||
               genre2 == Genre::Adventure;

    case Genre::Manga:
        return genre2 == Genre::Comics ||
               genre2 == Genre::Fantasy ||
               genre2 == Genre::Science_Fiction;

    // ===== علمی =====
    case Genre::Science:
        return genre2 == Genre::Technology ||
               genre2 == Genre::Psychology ||
               genre2 == Genre::Education;

    case Genre::Technology:
        return genre2 == Genre::Science ||
               genre2 == Genre::Education;

    case Genre::Psychology:
        return genre2 == Genre::Personal_Development ||
               genre2 == Genre::Health ||
               genre2 == Genre::Philosophy ||
               genre2 == Genre::Science;

    case Genre::Philosophy:
        return genre2 == Genre::Psychology ||
               genre2 == Genre::Politics_and_Society ||
               genre2 == Genre::History;

    // ===== تاریخی و اجتماعی =====
    case Genre::History:
        return genre2 == Genre::Biography ||
               genre2 == Genre::Politics_and_Society ||
               genre2 == Genre::Philosophy;

    case Genre::Biography:
        return genre2 == Genre::History ||
               genre2 == Genre::Documentation;

    case Genre::Politics_and_Society:
        return genre2 == Genre::History ||
               genre2 == Genre::Philosophy ||
               genre2 == Genre::Psychology;

    case Genre::Documentation:
        return genre2 == Genre::Biography ||
               genre2 == Genre::Education ||
               genre2 == Genre::Reference;

    // ===== آموزش =====
    case Genre::Education:
        return genre2 == Genre::Reference ||
               genre2 == Genre::Language_Learning ||
               genre2 == Genre::Science ||
               genre2 == Genre::Technology;

    case Genre::Language_Learning:
        return genre2 == Genre::Education ||
               genre2 == Genre::Reference;

    case Genre::Reference:
        return genre2 == Genre::Education ||
               genre2 == Genre::Documentation;

    // ===== سبک زندگی =====
    case Genre::Health:
        return genre2 == Genre::Psychology ||
               genre2 == Genre::Cooking ||
               genre2 == Genre::Personal_Development;

    case Genre::Cooking:
        return genre2 == Genre::Health;

    case Genre::Personal_Development:
        return genre2 == Genre::Psychology ||
               genre2 == Genre::Health ||
               genre2 == Genre::Education;

    // ===== هنر =====
    case Genre::Art:
        return genre2 == Genre::Music ||
               genre2 == Genre::Comics;

    case Genre::Music:
        return genre2 == Genre::Art ||
               genre2 == Genre::Biography;

    // ===== سایر =====
    case Genre::LGBTQ:
        return genre2 == Genre::Romance ||
               genre2 == Genre::Drama ||
               genre2 == Genre::Biography;

    case Genre::Poetry:
        return genre2 == Genre::Drama ||
               genre2 == Genre::Philosophy;

    case Genre::other:
        return false;

    default:
        return false;
    }
}