// notificationservice.h
#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include <QVector>
#include <QMap>
#include "../Shared/Notification.h"
#include "../Repositories/UserRepository.h"
#include "../Database/DatabaseInitializer.h"
#include "../Database/DatabaseManager.h"


class NotificationService  : public QObject{
    Q_OBJECT
private:
    QMap<int, QVector<Notification>> userNotifications;  // userId -> notifications
    int nextId = 1000;
    UserRepository* userRepo;

public:
    explicit NotificationService(UserRepository* repo , QObject* parent = nullptr);

    // ===== Create Notifications =====

    void setUserRepository(UserRepository* repo) { userRepo = repo; }

    void sendToUser(int userId, NotificationType type, const QString& title, const QString& message);

    void sendToAll(NotificationType type, const QString& title, const QString& message);


    void sendToRole(const QString& role, NotificationType type, const QString& title, const QString& message);

    // ===== Get Notifications =====

    QVector<Notification> getNotificationsForUser(int userId) const;


    QVector<Notification> getUnreadNotifications(int userId) const;

    Notification getNotificationById(int notificationId) const;

    // ===== Manage Notifications =====

    bool markAsRead(int notificationId, int userId);

    void markAllAsRead(int userId);

    bool deleteNotification(int notificationId, int userId);

    void clearAllNotifications(int userId);

    int getUnreadCount(int userId) const;



    bool loadAllFromDatabase();
    bool saveToDatabase(const Notification& notification);
    bool deleteFromDatabase(int notificationId);
    bool markAsReadInDatabase(int notificationId);

    static NotificationType stringToNotificationType(const QString& str);

    static QString notificationTypeToString(NotificationType type);


private:
    void addNotificationForUser(int userId, const Notification& notification);

    void addToCache(int userId, const Notification& notification);
    void clearCache();
};

#endif // NOTIFICATIONSERVICE_H