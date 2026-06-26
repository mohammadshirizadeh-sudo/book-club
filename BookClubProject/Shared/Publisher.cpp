// publisher.cpp
#include "Publisher.h"
#include <QDebug>

// ===== Constructors =====

Publisher::Publisher()
    : User()
    , publisherName("")
    , totalRevenue(0.0)
    , joinedAt(QDateTime::currentDateTime()) {
    setRole(UserRole::Publisher);
}

Publisher::Publisher(int id, const QString& username, const QString& email,
                     const QString& password, const QString& publisherName)
    : User(id, username, email, password)
    , publisherName(publisherName)
    , totalRevenue(0.0)
    , joinedAt(QDateTime::currentDateTime()) {
    setRole(UserRole::Publisher);
}

/*

Publisher::Publisher(int id, const QString& username, const QString& email,
                     const QString& passwordHash, const QString& salt,
                     const QString& publisherName)
    : User(id, username, email, passwordHash)
    , publisherName(publisherName)
    , totalRevenue(0.0)
    , joinedAt(QDateTime::currentDateTime()) {
    setRole(UserRole::Publisher);
}
*/

Publisher::Publisher(int id, const QString& fullName, const QString& username, const QString& email,
                     UserRole role, AccountStatus status,
                     const QDateTime& createdAt, const QDateTime& lastLogin,
                     const QString& passwordHash, const QVector<QString>& favouriteGenre,
                     const QDateTime& updatedAt, const QString& publisherName,
                     const QVector<int>& booksPublished, double totalRevenue , QString salt)
    : User(id, fullName, username, email, role, status,
           createdAt, lastLogin, passwordHash, favouriteGenre, updatedAt ,salt)  // ← فراخوانی Constructor پایه
    , publisherName(publisherName)
    , booksPublished(booksPublished)
    , totalRevenue(totalRevenue)
    , joinedAt(createdAt)  {

    // Publisher-specific initialization
    setRole(UserRole::Publisher);
}

// ===== Book Management =====

bool Publisher::addPublishedBook(int bookId) {
    if (bookId <= 0) {
        qWarning() << "Invalid book ID:" << bookId;
        return false;
    }

    if (hasPublishedBook(bookId)) {
        qWarning() << "Book" << bookId << "already published by this publisher";
        return false;
    }

    booksPublished.append(bookId);
    qDebug() << "Book" << bookId << "added to publisher's list";
    return true;
}

bool Publisher::removePublishedBook(int bookId) {
    if (!hasPublishedBook(bookId)) {
        qWarning() << "Book" << bookId << "not found in publisher's list";
        return false;
    }

    bool removed = booksPublished.removeOne(bookId);
    if (removed) {
        qDebug() << "Book" << bookId << "removed from publisher's list";
    }
    return removed;
}

bool Publisher::hasPublishedBook(int bookId) const {
    return booksPublished.contains(bookId);
}

// ===== Revenue Management =====

void Publisher::addRevenue(double amount) {
    if (amount < 0) {
        qWarning() << "Cannot add negative revenue:" << amount;
        return;
    }

    totalRevenue += amount;
    qDebug() << "Revenue added:" << amount << "Total:" << totalRevenue;
}

// ===== Statistics =====

QString Publisher::getFormattedRevenue() const {
    return QString("$%1").arg(totalRevenue, 0, 'f', 2);
}