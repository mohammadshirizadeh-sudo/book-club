// publisher.h
#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "User.h"
#include "Book.h"
#include <QString>
#include <QVector>
#include <QDateTime>


class Publisher : public User {
private:
    QString publisherName;

    double totalRevenue;
    QDateTime joinedAt;

public:
    // ===== Constructors =====
    Publisher();
    Publisher(int id, const QString& fullName, const QString& username, const QString& email,
              UserRole role, AccountStatus status,
              const QDateTime& createdAt, const QDateTime& lastLogin,
              const QString& passwordHash, const QVector<QString>& favouriteGenre,
              const QDateTime& updatedAt, const QString& publisherName,
               double totalRevenue , QString salt);

    Publisher(int id, const QString& username, const QString& email,
              const QString& password, const QString& publisherName);
    /*
    Publisher(int id, const QString& username, const QString& email,
              const QString& passwordHash, const QString& salt,
              const QString& publisherName);
*/



    // ===== Getters =====
    QString getPublisherName() const { return publisherName; }

    double getTotalRevenue() const { return totalRevenue; }
    QDateTime getJoinedAt() const { return joinedAt; }

    // ===== Setters =====
    void setPublisherName(const QString& name) { publisherName = name; }
    void setTotalRevenue(double revenue) { totalRevenue = revenue; }
    void setJoinedAt(const QDateTime& time) { joinedAt = time; }

    // ===== Book Management =====


    bool addPublishedBook(int bookId);
    bool removePublishedBook(int bookId);
    bool hasPublishedBook(int bookId) const;

    // ===== Revenue Management =====
    void addRevenue(double amount);
    void resetRevenue() { totalRevenue = 0.0; }

    // ===== Statistics =====

    QString getFormattedRevenue() const;
};

#endif // PUBLISHER_H