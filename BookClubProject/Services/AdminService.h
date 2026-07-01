#ifndef ADMINSERVICE_H
#define ADMINSERVICE_H
#include <QMap>
#include <QString>
#include<QVariant>
#include "UserService.h"
#include "BookService.h"
#include "ReviewService.h"
#include "PurchaseService.h"
#include<QVector>


// AdminService.h
class AdminService
{
public:
    QMap<QString, QVariant> getSystemStats() const;
    bool blockUser(int id , QString reason = "");
    bool unblockUser(int id);
    bool deleteUser(int id);
    QVector<User*>getAllUsers();
    QVector<User*> getBlockedUsers()const;
private:
    UserService* m_userService;
    BookService* m_bookService;
    ReviewService*m_reviewService;
    PurchaseService* m_purchaseService;

    //blockUser(userId, reason) unblockUser(userId)deleteUser(userId) getAllUsers() deleteBook(bookId, reason)getSystemStats()



};


#endif // ADMINSERVICE_H
