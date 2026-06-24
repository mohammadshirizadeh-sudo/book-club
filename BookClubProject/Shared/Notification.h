// notification.h
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QString>
#include <QDateTime>

/**
 * @brief Notification types
 */
enum class NotificationType {
    NewBook,
    Discount,
    NewSale,
    NewReview,
    System,
    Purchase,
    Promotional,
    Warning,
    Info
};

/**
 * @brief Notification class for system notifications
 */
class Notification {
private:
    int notificationId;
    int targetUserId;
    QString targetRole;
    NotificationType type;
    QString title;
    QString message;
    bool isRead;
    QDateTime createdAt;

public:

    Notification();
    Notification(int notificationId, int targetUserId,
                 NotificationType type, const QString& title, const QString& message);
    Notification(int notificationId, const QString& targetRole,
                 NotificationType type, const QString& title, const QString& message);


    int getNotificationId() const { return notificationId; }
    int getTargetUserId() const { return targetUserId; }
    QString getTargetRole() const { return targetRole; }
    NotificationType getType() const { return type; }
    QString getTitle() const { return title; }
    QString getMessage() const { return message; }
    bool getIsRead() const { return isRead; }
    QDateTime getCreatedAt() const { return createdAt; }

    void setNotificationId(int id) { notificationId = id; }
    void setTargetUserId(int id) { targetUserId = id; }
    void setTargetRole(const QString& role) { targetRole = role; }
    void setType(NotificationType type) { this->type = type; }
    void setTitle(const QString& title) { this->title = title; }
    void setMessage(const QString& message) { this->message = message; }
    void setCreatedAt(const QDateTime& time) { createdAt = time; }



    void markAsRead() { isRead = true; }


    void markAsUnread() { isRead = false; }

    //Helper Methods

    bool isForUser(int userId) const;

    bool isForRole(const QString& role) const;
    QString getTypeString() const;
\
    QString getDisplayText() const;
};

#endif // NOTIFICATION_H