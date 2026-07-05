#ifndef USER_H
#define USER_H
#include <QString>
#include <QDateTime>
#include <QVector>
#include "PasswordValidator.h"
#include"EmailValidator.h"
#include"ValidationResult.h"
#include "PasswordHelper.h"
#include "Genre.h"


enum class UserRole {
    User,
    Publisher,
    Admin
};
enum class AccountStatus {
    Active,
    Blocked,
    Inactive,
    Suspended,
    Pending
};

class User
{
private:
    int id;
    QString Fullname;
    QString username;
    QString passwordHash;
    QString email;

    UserRole role;
    AccountStatus status;
    QDateTime createdAt;
    QVector<Genre> favouriteGenre;
    QDateTime lastLogin;
    QDateTime updatedAt;
    QString salt;
    QString passwordResetToken;
    QDateTime resetTokenExpiry;



public:
    User(int id, const QString &fullName ,const QString& username, const QString& _email,UserRole role, AccountStatus status,
         const QDateTime& createdAt,const QDateTime& lastLogin , const QString& passwordHash , QVector<Genre> favouriteGenre,const QDateTime & updatedAt , QString salt);



public:
    //don't forgot default constructor
    User();
    User(int id, const QString& username, const QString& email, const QString& plainPassword);

    // 2. Constructor
    User(int id,const QString &fullName ,  const QString& username, const QString& email,
         UserRole role, AccountStatus status,
         const QDateTime& createdAt,const QDateTime& lastLogin ,const QString& plainPassword);
    int getId() const;
    void setId(int newId);
    QString getUsername() const;
    void setUsername(const QString &newUsername);
    QString getEmail() const;
    void setEmail(const QString &newEmail);
    bool isBlocked()const;
    bool checkPassword(const QString& _password)const;
    QDateTime getLastLogin() const;
    void setLastLogin(const QDateTime &newLastLogin);
    QString getFullname() const;
    void setFullname(const QString &newFullname);
    QVector<Genre> getFavouriteGenre() const;
    void setFavouriteGenre(const QVector<Genre> &newFavouriteGenre);
    QDateTime getUpdatedAt() const;
    void setUpdatedAt(const QDateTime &newUpdatedAt);
    bool setPassword(const QString& password);
    bool isAdmin()const;
    bool isPublisher()const;

    AccountStatus getStatus() const;
    void setStatus(AccountStatus newStatus);
    UserRole getRole() const;
    void setRole(UserRole newRole);


    QDateTime getCreatedAt() const;
    QString generateResetToken();



    //remember you can send real email for password
    bool verifyResetToken(const QString& token) const;


    bool resetPasswordWithToken(const QString& token, const QString& newPassword);

    void clearResetToken();

    bool isResetTokenExpired() const;

    // ===== Getters =====
    QString getPasswordResetToken() const { return passwordResetToken; }
    QDateTime getResetTokenExpiry() const { return resetTokenExpiry; }
    QString getRoleString() const;





    virtual ~User();

    void setPasswordResetToken(const QString &newPasswordResetToken);
    void setResetTokenExpiry(const QDateTime &newResetTokenExpiry);
    QString getPasswordHash() const;
    QString getSalt() const;
};

#endif // USER_H
