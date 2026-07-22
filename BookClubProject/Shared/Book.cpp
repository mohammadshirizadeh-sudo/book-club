// book.cpp
#include "Book.h"
#include <QDebug>

// ===== Constructors =====
Book::Book()
    : bookId(0)
    , title("")
    , author("")
    , genre()
    , description("")
    , price(0.0)
    , discountPercent(0.0)
    , isTimedDiscount(false)
    , discountStartDate(QDateTime())
    , discountEndDate(QDateTime())

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
           const Genre& genre, double price, int publisherId)
    : bookId(bookId)
    , title(title)
    , author(author)
    , genre(genre)
    , description("")
    , price(price)
    , discountPercent(0.0)
    , isTimedDiscount(false)
    , discountStartDate(QDateTime())
    , discountEndDate(QDateTime())
    , coverPath("")
    , pdfPath("")
    , isActive(true)
    , averageRating(0.0)
    , salesCount(0)
    , publisherId(publisherId)
    , createdAt(QDateTime::currentDateTime())
    , updatedAt(QDateTime::currentDateTime()) {
}



Book::Book(int bookId, const QString& title, const QString& author,
           const Genre& genre, const QString& description, double price,
           double discountPercent,bool isTimedDiscount,
           const QDateTime& discountStartDate, const QDateTime& discountEndDate,
           const QString& coverPath, const QString& pdfPath,
           bool isActive, double averageRating, int salesCount, int publisherId,
           const QDateTime& createdAt, const QDateTime& updatedAt)
    : bookId(bookId)
    , title(title)
    , author(author)
    , genre(genre)
    , description(description)
    , price(price)
    , discountPercent(discountPercent)
    , isTimedDiscount(isTimedDiscount)
    , discountStartDate(discountStartDate)
    , discountEndDate(discountEndDate)
    , coverPath(coverPath)
    , pdfPath(pdfPath)
    , isActive(isActive)
    , averageRating(averageRating)
    , salesCount(salesCount)
    , publisherId(publisherId)
    , createdAt(createdAt)
    , updatedAt(updatedAt)
{
}


// ===== Core Methods =====

void Book::applyDiscount(double percent)
{
    if (percent < 0.0 || percent > 100.0) {
        qWarning() << "Invalid discount percent:" << percent;
        return;
    }

    discountPercent = percent;
    isTimedDiscount = false;
    discountStartDate = QDateTime();
    discountEndDate = QDateTime();
    updatedAt = QDateTime::currentDateTime();

    qDebug() << "Discount applied to book" << title << ":" << percent << "%";
}

void Book::applyTimedDiscount(double percent, const QDateTime& startDate, const QDateTime& endDate)
{
    if (percent < 0.0 || percent > 100.0) {
        qWarning() << "Invalid discount percent:" << percent;
        return;
    }

    if (!startDate.isValid() || !endDate.isValid()) {
        qWarning() << "Invalid start or end date for timed discount";
        return;
    }

    if (startDate >= endDate) {
        qWarning() << "Start date must be before end date";
        return;
    }

    discountPercent = percent;
    isTimedDiscount = true;
    discountStartDate = startDate;
    discountEndDate = endDate;
    updatedAt = QDateTime::currentDateTime();

    qDebug() << "Timed discount applied to book" << title << ":" << percent << "%"
             << "from" << startDate.toString() << "to" << endDate.toString();
}

void Book::removeDiscount()
{
    if (discountPercent > 0.0 || isTimedDiscount) {
        discountPercent = 0.0;
        isTimedDiscount = false;
        discountStartDate = QDateTime();
        discountEndDate = QDateTime();
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

double Book::getFinalPrice() const
{
    double finalPrice = price * (1.0 - discountPercent / 100.0);

    // اگر تخفیف زمان‌دار است و زمان آن تمام شده، قیمت اصلی برگردانده شود
    if (isTimedDiscount && discountEndDate.isValid()) {
        if (QDateTime::currentDateTime() > discountEndDate) {
            return price;  // تخفیف منقضی شده
        }
    }

    return finalPrice;
}


bool Book::isFree() const {
    return price < std::numeric_limits<double>::epsilon();//the reason i don't wirte price==0.0 is that floating-point equality is unsafe
}

bool Book::isDiscounted() const
{
    if (isTimedDiscount && discountEndDate.isValid()) {
        if (QDateTime::currentDateTime() > discountEndDate) {
            return false;  // تخفیف منقضی شده
        }
    }
    return discountPercent > 0.0;
}