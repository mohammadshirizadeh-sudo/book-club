// notification.cpp
#include "Notification.h"
#include <QDebug>


Notification::Notification()
    : notificationId(0)
    , targetUserId(0)
    , targetRole("All")
    , type(NotificationType::Info)
    , title("")
    , message("")
    , isRead(false)
    , createdAt(QDateTime::currentDateTime()) {
}

Notification::Notification(int notificationId, int targetUserId,
                           NotificationType type, const QString& title, const QString& message)
    : notificationId(notificationId)
    , targetUserId(targetUserId)
    , targetRole("")
    , type(type)
    , title(title)
    , message(message)
    , isRead(false)
    , createdAt(QDateTime::currentDateTime()) {
}

Notification::Notification(int notificationId, const QString& targetRole,
                           NotificationType type, const QString& title, const QString& message)
    : notificationId(notificationId)
    , targetUserId(0)
    , targetRole(targetRole)
    , type(type)
    , title(title)
    , message(message)
    , isRead(false)
    , createdAt(QDateTime::currentDateTime()) {
}



bool Notification::isForUser(int userId) const {
    if (targetUserId == 0) {
        return true;  // All users
    }
    if (targetUserId == -1) {
        return false; // Role-based only
    }
    return targetUserId == userId;
}

bool Notification::isForRole(const QString& role) const {
    if (targetRole.isEmpty() || targetRole == "All") {
        return true;  // All roles
    }
    return targetRole == role;
}

QString Notification::getTypeString() const {
    switch(type) {
    case NotificationType::NewBook:    return "New Book";
    case NotificationType::Discount:   return "Discount";
    case NotificationType::NewSale:    return "New Sale";
    case NotificationType::NewReview:  return "New Review";
    case NotificationType::System:     return "System";
    case NotificationType::Purchase:   return "Purchase";
    case NotificationType::Promotional:return "Promotional";
    case NotificationType::Warning:    return "Warning";
    case NotificationType::Info:       return "Info";
    default: return "Unknown";
    }
}

QString Notification::getDisplayText() const {
    QString status = isRead ? "[✓] " : "[○] ";
    return status + "[" + getTypeString() + "] " + title + ": " + message;
}