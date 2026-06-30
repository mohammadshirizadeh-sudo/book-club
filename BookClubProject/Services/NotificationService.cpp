// notificationservice.cpp
#include "NotificationService.h"
#include <QDebug>

NotificationService::NotificationService(UserRepository* repo) :userRepo(repo) {
}

void NotificationService::sendToUser(int userId, NotificationType type,
                                     const QString& title, const QString& message) {
    Notification notification(nextId++, userId, type, title, message);
    addNotificationForUser(userId, notification);
    qDebug() << "Notification sent to user" << userId << ":" << title;
}

void NotificationService::sendToAll(NotificationType type,
                                    const QString& title, const QString& message) {
    // Would need list of all users here
    // For now, just demo: send to user 0 as "All"
    if (!userRepo) {
        qWarning() << "UserRepository not set! Cannot send to all users.";
        return;
    }
    QVector<User*> allUsers = userRepo->getAllUsers();
    for (User* user: allUsers){
        Notification notification(nextId++, user->getId(), type, title, message);
        addNotificationForUser(user->getId(),  notification);
    }


    // In real implementation, loop through all users
    qDebug() << "Notification sent to all:" << title;
}

void NotificationService::sendToRole(const QString& role, NotificationType type,
                                     const QString& title, const QString& message) {
    if (!userRepo) {
        qWarning() << "UserRepository not set! Cannot send to role.";
        return;
    }
    QVector<User*> allUsers = userRepo->getAllUsers();
    int count =0;
    for (User* user : allUsers){
        if (role == "Admin" && user->isAdmin()) {
            Notification notification(nextId++, user->getId(), type, title, message);
            addNotificationForUser(user->getId(), notification);
            count++;
        } else if (role == "Publisher" && user->isPublisher()) {
            Notification notification(nextId++, user->getId(), type, title, message);
            addNotificationForUser(user->getId(), notification);
            count++;
        } else if (role == "User" && !user->isAdmin() && !user->isPublisher()) {
            Notification notification(nextId++, user->getId(), type, title, message);
            addNotificationForUser(user->getId(), notification);
            count++;
        } else if (role == "All") {
            // همه کاربران (همان sendToAll)
            Notification notification(nextId++, user->getId(), type, title, message);
            addNotificationForUser(user->getId(), notification);
            count++;
        }
    }

    // Would need list of users with this role here
    // For now, just demo
    qDebug() << "Notification sent to" << count << "users with role" << role << ":" << title;
}

void NotificationService::addNotificationForUser(int userId, const Notification& notification) {
    if (!userNotifications.contains(userId)) {
        userNotifications[userId] = QVector<Notification>();
    }
    userNotifications[userId].append(notification);
}

QVector<Notification> NotificationService::getNotificationsForUser(int userId) const {
    if (userNotifications.contains(userId)) {
        return userNotifications[userId];
    }
    return QVector<Notification>();
}

QVector<Notification> NotificationService::getUnreadNotifications(int userId) const {
    QVector<Notification> unread;
    if (userNotifications.contains(userId)) {
        for (const Notification& notif : userNotifications[userId]) {
            if (!notif.getIsRead()) {
                unread.append(notif);
            }
        }
    }
    return unread;
}

Notification NotificationService::getNotificationById(int notificationId) const {
    for (const auto& list : userNotifications) {
        for (const Notification& notif : list) {
            if (notif.getNotificationId() == notificationId) {
                return notif;
            }
        }
    }
    return Notification();
}

bool NotificationService::markAsRead(int notificationId, int userId) {
    if (!userNotifications.contains(userId)) {
        return false;
    }

    for (Notification& notif : userNotifications[userId]) {
        if (notif.getNotificationId() == notificationId) {
            notif.markAsRead();
            return true;
        }
    }
    return false;
}

void NotificationService::markAllAsRead(int userId) {
    if (!userNotifications.contains(userId)) {
        return;
    }

    for (Notification& notif : userNotifications[userId]) {
        notif.markAsRead();
    }
}

bool NotificationService::deleteNotification(int notificationId, int userId) {
    if (!userNotifications.contains(userId)) {
        return false;
    }

    QVector<Notification>& notifications = userNotifications[userId];
    for (int i = 0; i < notifications.size(); ++i) {
        if (notifications[i].getNotificationId() == notificationId) {
            notifications.remove(i);
            return true;
        }
    }
    return false;
}

void NotificationService::clearAllNotifications(int userId) {
    if (userNotifications.contains(userId)) {
        userNotifications[userId].clear();
    }
}

int NotificationService::getUnreadCount(int userId) const {
    return getUnreadNotifications(userId).size();
}