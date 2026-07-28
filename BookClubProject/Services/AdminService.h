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
#include "../Server/server.h"
#include "AccessLog.h"
#include<QVector>


struct AccessLogRecord {
    QDateTime timestamp;
    QString adminName;
    QString action;
    QString targetUser;
    QString ipAddress;
    QString status;
};
// AdminService.h
class AdminService : public QObject
{
public:
    explicit AdminService(UserService* userService,
                          BookService* bookService,
                          ReviewService* reviewService,
                          PurchaseService* purchaseService,
                          NotificationService* notifService,
                          LibraryService* libraryService,
                          Server* server = nullptr,
                          QObject* parent = nullptr);
    QMap<QString, QVariant> getSystemStats() const;
    bool blockUser(int id , const QString& reason);
    bool unblockUser(int id);
    bool deleteUser(int id);
    QVector<User*> getBlockedUsers()const;

    QVector<QVariantMap> getRecentActivities(int limit = 50) const;
    QStringList getSystemAlerts() const;
    QMap<QString, QVariant> getDatabaseStatus() const;
    void appendAccessLog(const AccessLogEntry& record);
    QVector<AccessLogEntry> getAccessLogs() const;


    void clearAccessLogs();

    int getOnlineUserCount() const;
    bool isDatabaseConnected() const;
    QString getServerUptimeString() const;


    QString backupDatabase() const;
    void clearCaches();
    void scheduleRestart(int delayMs = 1000);
    void cancelRestart();
    void loadAccessLogsFromDatabase();

private:
    UserService* m_userService;
    BookService* m_bookService;
    ReviewService*m_reviewService;
    PurchaseService* m_purchaseService;
    NotificationService* m_notifService;
    LibraryService* m_libraryService;

    QVector<AccessLogEntry> m_accessLog;
    mutable QMutex m_logMutex;
    Server* m_server;


    //blockUser(userId, reason) unblockUser(userId)deleteUser(userId) getAllUsers() deleteBook(bookId, reason)getSystemStats()



};


#endif // ADMINSERVICE_H
