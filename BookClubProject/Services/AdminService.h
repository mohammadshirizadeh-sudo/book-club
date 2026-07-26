#ifndef ADMINSERVICE_H
#define ADMINSERVICE_H
#include <QMap>
#include <QString>
#include<QVariant>
#include "UserService.h"
#include "BookService.h"
#include "ReviewService.h"
#include "PurchaseService.h"
#include "NotificationService.h"
#include<QVector>


// AdminService.h
class AdminService : public QObject
{
public:
    explicit AdminService(UserService* userService,
                          BookService* bookService,
                          ReviewService* reviewService,
                          PurchaseService* purchaseService,
                          NotificationService* notifService,
                          QObject* parent = nullptr);
    QMap<QString, QVariant> getSystemStats() const;
    bool blockUser(int id , QString reason = "");
    bool unblockUser(int id);
    bool deleteUser(int id);
    QVector<User*>getAllUsers();
    QVector<User*> getBlockedUsers()const;

    QStringList getRecentActivities(int limit = 50) const;
    QStringList getSystemAlerts() const;
    QMap<QString, QVariant> getServerStatus() const;
    QMap<QString, QVariant> getDatabaseStatus() const;
private:
    UserService* m_userService;
    BookService* m_bookService;
    ReviewService*m_reviewService;
    PurchaseService* m_purchaseService;
    NotificationService* m_notifService;

    //blockUser(userId, reason) unblockUser(userId)deleteUser(userId) getAllUsers() deleteBook(bookId, reason)getSystemStats()



};


#endif // ADMINSERVICE_H
