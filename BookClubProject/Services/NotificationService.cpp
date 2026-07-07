// notificationservice.cpp
#include "NotificationService.h"
#include <QDebug>

NotificationService::NotificationService(UserRepository* repo , QObject* parent) :userRepo(repo), QObject(parent) {
    loadAllFromDatabase();
}

void NotificationService::sendToUser(int userId, NotificationType type,
                                     const QString& title, const QString& message) {
    Notification notification(nextId++, userId, type, title, message);

    addToCache(userId, notification);

    // 2. Save to SQLite
    saveToDatabase(notification);

}

void NotificationService::sendToAll(NotificationType type,
                                    const QString& title, const QString& message) {
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
                                     const QString& title, const QString& message)
{
    if (!userRepo) {
        qWarning() << "UserRepository not set! Cannot send to role.";
        return;
    }

    QVector<User*> allUsers = userRepo->getAllUsers();
    int count = 0;

    for (User* user : allUsers) {
        bool shouldSend = false;

        if (role == "Admin" && user->isAdmin()) {
            shouldSend = true;
        } else if (role == "Publisher" && user->isPublisher()) {
            shouldSend = true;
        } else if (role == "User" && !user->isAdmin() && !user->isPublisher()) {
            shouldSend = true;
        } else if (role == "All") {
            shouldSend = true;
        }

        if (shouldSend) {
            Notification notification(nextId++, user->getId(), type, title, message);
            addToCache(user->getId(), notification);
            saveToDatabase(notification);
            count++;
        }
    }

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

            markAsReadInDatabase(notificationId);
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
        markAsReadInDatabase(notif.getNotificationId());
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
            deleteFromDatabase(notificationId);
            return true;
        }
    }
    return false;
}

void NotificationService::clearAllNotifications(int userId)
{
    if (userNotifications.contains(userId)) {
        // ✅ Delete all from SQLite
        for (const Notification& notif : userNotifications[userId]) {
            deleteFromDatabase(notif.getNotificationId());
        }
        userNotifications[userId].clear();
    }
}

int NotificationService::getUnreadCount(int userId) const {
    return getUnreadNotifications(userId).size();
}




bool NotificationService::loadAllFromDatabase()
{
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT id, user_id, type, title, message, is_read, created_at
        FROM notification
        ORDER BY created_at DESC
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load notifications:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        Notification notification(
            sqlQuery.value("id").toInt(),
            sqlQuery.value("user_id").toInt(),
            stringToNotificationType(sqlQuery.value("type").toString()),
            sqlQuery.value("title").toString(),
            sqlQuery.value("message").toString(),
            sqlQuery.value("is_read").toBool(),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate)
            );

        addToCache(notification.getTargetUserId(), notification);
        count++;
    }
    qDebug() << "✅ Loaded" << count << "notifications from SQLite";
    return true;
}

bool NotificationService::saveToDatabase(const Notification& notification)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO notification (
            id, user_id, type, title, message, is_read, created_at
        ) VALUES (
            :id, :user_id, :type, :title, :message, :is_read, :created_at
        )
    )";

    QVariantMap params;
    params["id"] = notification.getNotificationId();
    params["user_id"] = notification.getTargetUserId();
    params["type"] = notificationTypeToString(notification.getType());
    params["title"] = notification.getTitle();
    params["message"] = notification.getMessage();
    params["is_read"] = notification.getIsRead() ? 1 : 0;
    params["created_at"] = notification.getCreatedAt().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}

bool NotificationService::deleteFromDatabase(int notificationId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = "DELETE FROM notification WHERE id = :id";
    QVariantMap params;
    params["id"] = notificationId;

    return db->executeQuery(query, params);
}

bool NotificationService::markAsReadInDatabase(int notificationId)
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = "UPDATE notification SET is_read = 1 WHERE id = :id";
    QVariantMap params;
    params["id"] = notificationId;

    return db->executeQuery(query, params);
}

// =============================================
// ===== Helper Methods =====
// =============================================

void NotificationService::addToCache(int userId, const Notification& notification)
{
    if (!userNotifications.contains(userId)) {
        userNotifications[userId] = QVector<Notification>();
    }
    userNotifications[userId].append(notification);
}

void NotificationService::clearCache()
{
    userNotifications.clear();
}



// =============================================
// ===== Converters =====
// =============================================

NotificationType NotificationService :: stringToNotificationType(const QString& str)
{
    if (str == "NewBook") return NotificationType::NewBook;
    if (str == "Discount") return NotificationType::Discount;
    if (str == "NewSale") return NotificationType::NewSale;
    if (str == "NewReview") return NotificationType::NewReview;
    if (str == "System") return NotificationType::System;
    if (str == "Purchase") return NotificationType::Purchase;
    if (str == "Promotional") return NotificationType::Promotional;
    if (str == "Warning") return NotificationType::Warning;
    return NotificationType::Info;
}

QString NotificationService::notificationTypeToString(NotificationType type)
{
    switch(type) {
    case NotificationType::NewBook: return "NewBook";
    case NotificationType::Discount: return "Discount";
    case NotificationType::NewSale: return "NewSale";
    case NotificationType::NewReview: return "NewReview";
    case NotificationType::System: return "System";
    case NotificationType::Purchase: return "Purchase";
    case NotificationType::Promotional: return "Promotional";
    case NotificationType::Warning: return "Warning";
    default: return "Info";
    }
}