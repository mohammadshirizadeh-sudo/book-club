// book.h
#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QDateTime>

class Book {
private:
    int bookId;
    QString title;
    QString author;
    QString genre;
    QString description;
    double price;
    double discountPercent;
    QString coverPath;
    QString pdfPath;
    bool isActive;
    double averageRating;
    int salesCount;
    int publisherId;
    QDateTime createdAt;
    QDateTime updatedAt;

public:

    Book();
    Book(int bookId, const QString& title, const QString& author,
         const QString& genre, double price, int publisherId);


    int getBookId() const { return bookId; }
    QString getTitle() const { return title; }
    QString getAuthor() const { return author; }
    QString getGenre() const { return genre; }
    QString getDescription() const { return description; }
    double getPrice() const { return price; }
    double getDiscountPercent() const { return discountPercent; }
    QString getCoverPath() const { return coverPath; }
    QString getPdfPath() const { return pdfPath; }
    bool getIsActive() const { return isActive; }
    double getAverageRating() const { return averageRating; }
    int getSalesCount() const { return salesCount; }
    int getPublisherId() const { return publisherId; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getUpdatedAt() const { return updatedAt; }


    void setBookId(int id) { bookId = id; }
    void setTitle(const QString& title) { this->title = title; }
    void setAuthor(const QString& author) { this->author = author; }
    void setGenre(const QString& genre) { this->genre = genre; }
    void setDescription(const QString& desc) { description = desc; }
    void setPrice(double price) { this->price = price; }
    void setCoverPath(const QString& path) { coverPath = path; }
    void setPdfPath(const QString& path) { pdfPath = path; }
    void setAverageRating(double rating) { averageRating = rating; }
    void setSalesCount(int count) { salesCount = count; }
    void setPublisherId(int id) { publisherId = id; }
    void setCreatedAt(const QDateTime& time) { createdAt = time; }
    void setUpdatedAt(const QDateTime& time) { updatedAt = time; }



    void applyDiscount(double percent);

    void removeDiscount();

    void activate();

    void deactivate();

    double getFinalPrice() const;

    bool isFree() const;

    bool isDiscounted() const;
};

#endif // BOOK_H