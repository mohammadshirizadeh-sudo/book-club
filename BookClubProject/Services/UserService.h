// userservice.h
#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <QString>
#include <QVector>
#include "../Shared/User.h"
#include "../Repositories/UserRepository.h"

class UserService : public QObject {
    Q_OBJECT
private:
    UserRepository* userRepo;

public:
    // ===== Constructor =====
    explicit UserService(UserRepository* repo , QObject* parent = nullptr);

    // ===== Profile Management =====
    User* getProfile(int userId) const;
    QString getStringStatus(int userId)const;
    QString getStringStatus(AccountStatus status);


    // bool updateProfile(int userId, const QString& newEmail,
    //                    const QString& newFullName,
    //                    const QVector<Genre>& newGenres);

    ValidationResult updateProfile(int userId, const QString& newEmail,
                                    const QString& newFullName,
                       const QString& newUserName);

    bool updateFavoriteGenres(int userId, const QVector<Genre>& newGenres);

    bool changePassword(int userId, const QString& oldPassword, const QString& newPassword);


    bool blockUser(int userId, const QString& reason = "");

    bool unblockUser(int userId);
    bool deleteUser(int userId);

    // ===== Helper Methods =====
    QVector<User*> getAllUsers() const;
    QVector<User*> getBlockedUsers() const;
    QVector<User*> searchUsers(const QString& keyword) const;
    bool isUsernameAvailable(const QString& username) const;
    bool isEmailAvailable(const QString& email) const;
    bool addFavoriteBook(int userId, int bookId);
    bool removeFavoriteBook(int userId, int bookId);
    bool isFavoriteBook(int userId, int bookId) const;

    QVector<int> getFavoriteBooks(int userId) const;
};

#endif // USERSERVICE_H