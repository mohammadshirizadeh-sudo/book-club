// notification.h
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QString>
#include <QDateTime>

enum class NotificationType {
    NewBook,        // New book published in favorite genre
    Discount,       // Discount on saved/bookmarked book
    NewSale,        // New sale for publisher
    NewReview,      // New review/rating for publisher's book
    System,         // System notification
    Purchase,       // Purchase confirmation
    Promotional,    // Promotional message
    Warning,        // Warning message
    Info            // General information
};

class Notification {
private:
    int notificationId;
    int targetUserId;          // 0 = all users, -1 = specific role
    QString targetRole;        // "User", "Publisher", "Admin", "All"
    NotificationType type;
    QString title;
    QString message;
    bool isRead;
    QDateTime createdAt;

public:
    // ===== Constructors =====
    Notification();
    Notification(int notificationId, int targetUserId,
                 NotificationType type, const QString& title, const QString& message);
    Notification(int notificationId, const QString& targetRole,
                 NotificationType type, const QString& title, const QString& message);

    // ===== Getters =====
    int getNotificationId() const { return notificationId; }
    int getTargetUserId() const { return targetUserId; }
    QString getTargetRole() const { return targetRole; }
    NotificationType getType() const { return type; }
    QString getTitle() const { return title; }
    QString getMessage() const { return message; }
    bool getIsRead() const { return isRead; }
    QDateTime getCreatedAt() const { return createdAt; }

    // ===== Setters =====
    void setNotificationId(int id) { notificationId = id; }
    void setTargetUserId(int id) { targetUserId = id; }
    void setTargetRole(const QString& role) { targetRole = role; }
    void setType(NotificationType type) { this->type = type; }
    void setTitle(const QString& title) { this->title = title; }
    void setMessage(const QString& message) { this->message = message; }
    void setCreatedAt(const QDateTime& time) { createdAt = time; }

    // ===== Core Methods =====

    void markAsRead() { isRead = true; }
    void markAsUnread() { isRead = false; }

    // ===== Helper Methods =====
    bool isForUser(int userId) const;
    bool isForRole(const QString& role) const;
    QString getTypeString() const;

    QString getDisplayText() const;
};

#endif // NOTIFICATION_H