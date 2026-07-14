// userservice.cpp
#include "UserService.h"
#include "../Shared/EmailValidator.h"
#include "../Shared/PasswordValidator.h"
#include "../Shared/PasswordHelper.h"
#include <QDebug>

// ===== Constructor =====
UserService::UserService(UserRepository* repo ,QObject* parent)
    : userRepo(repo) , QObject(parent) {
}

// ===== Profile Management =====

User* UserService::getProfile(int userId) const {
    if (userId <= 0) {
        qWarning() << "Invalid user ID:" << userId;
        return nullptr;
    }

    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return nullptr;
    }

    return user;

}

bool UserService::updateProfile(int userId, const QString& newEmail,
                                const QString& newFullName,
                                const QString& newUserName) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    QString oldUserName =  user->getUsername();
    QString oldEmail = user->getEmail();
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Validate email
    ValidationResult email = EmailValidator::isValid(newEmail);
    if (!email.isValid) {
        qWarning() << "Invalid email:" << email.errorMessage;
        return false;
    }

    // 3. Check if email is taken by another user
    User* userWithEmail = userRepo->findByEmail(newEmail);
    if (userWithEmail && userWithEmail->getId() != userId) {
        qWarning() << "Email" << newEmail << "is already taken by another user";
        return false;
    }


    // 5. Update user data
    user->setEmail(newEmail);
    user->setFullname(newFullName);
    user->setUsername(newUserName);
    user->setUpdatedAt(QDateTime::currentDateTime());

    // 6. Save to repository
    if (!userRepo->updateUser(user , oldUserName , oldEmail)) {
        qWarning() << "Failed to update user in repository";
        return false;
    }

    qDebug() << "Profile updated for user:" << user->getUsername();
    return true;
}

bool UserService::updateFavoriteGenres(int userId, const QVector<Genre>& newGenres) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Validate genres (1-3 genres)
    if (newGenres.isEmpty() || newGenres.size() > 3) {
        qWarning() << "Invalid number of genres. Must be 1-3.";
        return false;
    }

    // 3. Update genres
    user->setFavouriteGenre(newGenres);
    user->setUpdatedAt(QDateTime::currentDateTime());

    // 4. Save to repository
    return userRepo->updateUser(user , user->getUsername(), user->getEmail());
}

bool UserService::changePassword(int userId, const QString& oldPassword, const QString& newPassword) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Check if user is blocked
    if (user->isBlocked()) {
        qWarning() << "User is blocked, cannot change password";
        return false;
    }

    // 3. Verify old password
    if (!user->checkPassword(oldPassword)) {
        qWarning() << "Old password is incorrect";
        return false;
    }

    // 4. Validate new password

    ValidationResult pass = PasswordValidator::isValid(newPassword);
    if (!pass.isValid) {
        qWarning() << "Invalid new password:" << pass.errorMessage;
        return false;
    }

    // 5. Set new password
    if (!user->setPassword(newPassword)) {
        qWarning() << "Failed to set new password";
        return false;
    }

    user->setUpdatedAt(QDateTime::currentDateTime());

    // 6. Save to repository
    if (!userRepo->updateUser(user , user->getUsername(), user->getEmail())) {
        qWarning() << "Failed to update user in repository";
        return false;
    }

    qDebug() << "Password changed for user:" << user->getUsername();
    return true;
}

// ===== User Management (Admin Functions) =====

bool UserService::blockUser(int userId, const QString& reason) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Cannot block an admin
    if (user->isAdmin()) {
        qWarning() << "Cannot block an admin!";
        return false;
    }

    // 3. Check if already blocked
    if (user->isBlocked()) {
        qWarning() << "User is already blocked!";
        return false;
    }

    // 4. Block the user
    user->setStatus(AccountStatus::Blocked);
    user->setUpdatedAt(QDateTime::currentDateTime());

    // 5. Save to repository
    if (!userRepo->updateUser(user , user->getUsername(), user->getEmail())) {
        qWarning() << "Failed to update user in repository";
        return false;
    }

    qDebug() << "User blocked:" << user->getUsername() << "Reason:" << reason;
    return true;
}

bool UserService::unblockUser(int userId) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Check if user is actually blocked
    if (!user->isBlocked()) {
        qWarning() << "User is not blocked!";
        return false;
    }

    // 3. Unblock the user
    user->setStatus(AccountStatus::Active);
    user->setUpdatedAt(QDateTime::currentDateTime());

    // 4. Save to repository
    if (!userRepo->updateUser(user , user->getUsername(), user->getEmail())) {
        qWarning() << "Failed to update user in repository";
        return false;
    }

    qDebug() << "User unblocked:" << user->getUsername();
    return true;
}

bool UserService::deleteUser(int userId) {
    // 1. Find user
    User* user = userRepo->findById(userId);
    if (!user) {
        qWarning() << "User not found with ID:" << userId;
        return false;
    }

    // 2. Cannot delete an admin
    if (user->isAdmin()) {
        qWarning() << "Cannot delete an admin!";
        return false;
    }

    // 3. Delete from repository
    if (!userRepo->deleteUser(userId)) {
        qWarning() << "Failed to delete user from repository";
        return false;
    }

    qDebug() << "User deleted:" << user->getUsername() << "(ID:" << userId << ")";
    return true;
}

// ===== Helper Methods =====

QVector<User*> UserService::getAllUsers() const {
    return userRepo->getAllUsers();
}

QVector<User*> UserService::getBlockedUsers() const {
    QVector<User*> blocked;
    for (User* user : userRepo->getAllUsers()) {
        if (user->isBlocked()) {
            blocked.append(user);
        }
    }
    return blocked;
}

QVector<User*> UserService::searchUsers(const QString& keyword) const {
    if (keyword.isEmpty()) {
        return QVector<User*>();
    }

    QVector<User*> results;
    QString lowerKeyword = keyword.toLower();

    for (User* user : userRepo->getAllUsers()) {
        if (user->getUsername().toLower().contains(lowerKeyword) ||
            user->getEmail().toLower().contains(lowerKeyword) ||
            user->getFullname().toLower().contains(lowerKeyword)) {
            results.append(user);
        }
    }

    return results;
}

bool UserService::isUsernameAvailable(const QString& username) const {
    return !userRepo->isUsernameTaken(username);
}

bool UserService::isEmailAvailable(const QString& email) const {
    return !userRepo->isEmailTaken(email);
}


// UserService.cpp
QString UserService::getStringStatus(AccountStatus status)
{
    switch(status) {
    case AccountStatus::Active:   return "Active";
    case AccountStatus::Blocked:  return "Blocked";
    case AccountStatus::Inactive: return "Inactive";
    case AccountStatus::Suspended:return "Suspended";
    case AccountStatus::Pending:  return "Pending";
    default: return "Unknown";
    }
}