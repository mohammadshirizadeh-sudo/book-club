// book.cpp
#include "Book.h"
#include <QDebug>

// ===== Constructors =====
Book::Book()
    : bookId(0)
    , title("")
    , author("")
    , genre("")
    , description("")
    , price(0.0)
    , discountPercent(0.0)
    , coverPath("")
    , pdfPath("")
    , isActive(true)
    , averageRating(0.0)
    , salesCount(0)
    , publisherId(0)
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime()) {
}

Book::Book(int bookId, const QString& title, const QString& author,
           const QString& genre, double price, int publisherId)
    : bookId(bookId)
    , title(title)
    , author(author)
    , genre(genre)
    , description("")
    , price(price)
    , discountPercent(0.0)
    , coverPath("")
    , pdfPath("")
    , isActive(true)
    , averageRating(0.0)
    , salesCount(0)
    , publisherId(publisherId)
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime()) {
}

// ===== Core Methods =====

void Book::applyDiscount(double percent) {
    if (percent < 0.0 || percent > 100.0) {
        qWarning() << "Invalid discount percent:" << percent;
        return;
    }

    discountPercent = percent;
    updatedAt = QDateTime::currentDateTime();

    qDebug() << "Discount applied to book" << title << ":" << percent << "%";
}

void Book::removeDiscount() {
    if (discountPercent > 0.0) {
        discountPercent = 0.0;
        updatedAt = QDateTime::currentDateTime();
        qDebug() << "Discount removed from book" << title;
    }
}

void Book::activate() {
    if (!isActive) {
        isActive = true;
        updatedAt = QDateTime::currentDateTime();
        qDebug() << "Book activated:" << title;
    }
}

void Book::deactivate() {
    if (isActive) {
        isActive = false;
        updatedAt = QDateTime::currentDateTime();
        qDebug() << "Book deactivated:" << title;
    }
}

// ===== Helper Methods =====

double Book::getFinalPrice() const {
    return price * (1.0 - discountPercent / 100.0);
}

bool Book::isFree() const {
    return price < std::numeric_limits<double>::epsilon();//the reason i don't wirte price==0.0 is that floating-point equality is unsafe
}

bool Book::isDiscounted() const {
    return discountPercent > 0.0;
}