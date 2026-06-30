// notificationservice.h
#ifndef NOTIFICATIONSERVICE_H
#define NOTIFICATIONSERVICE_H

#include <QVector>
#include <QMap>
#include "../Shared/Notification.h"
#include "../Repositories/UserRepository.h"

/**
 * @brief Notification Service for managing user notifications
 */
class NotificationService {
private:
    QMap<int, QVector<Notification>> userNotifications;  // userId -> notifications
    int nextId = 1000;
    UserRepository* userRepo;

public:
    explicit NotificationService(UserRepository* repo);

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

private:
    void addNotificationForUser(int userId, const Notification& notification);
};

#endif // NOTIFICATIONSERVICE_H