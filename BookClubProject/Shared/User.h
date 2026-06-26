#ifndef USER_H
#define USER_H
#include <QString>
#include <QDateTime>
#include <QVector>
#include "PasswordValidator.h"
#include"EmailValidator.h"
#include "PasswordHelper.h"


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
    QString passwordHash;//we have a header file("PasswordValidator") for management password
    QString email;

    UserRole role;
    AccountStatus status;
    QDateTime createdAt;
    QVector<QString> favouriteGenre;//for this we have a header file "Genre.h"
    QDateTime lastLogin;
    QDateTime updatedAt;
    QString salt;



protected:
    User(int id, const QString &fullName ,const QString& username, const QString& _email,UserRole role, AccountStatus status,
         const QDateTime& createdAt,const QDateTime& lastLogin , const QString& passwordHash , QVector<QString> favouriteGenre,const QDateTime & updatedAt , QString salt);

public:
    //don't forgot default constructor
    User();
    User(int id, const QString& username, const QString& email, const QString& plainPassword);

    // 2. Constructor
    User(int id,const QString &fullName ,  const QString& username, const QString& email,
         UserRole role, AccountStatus status,
         const QDateTime& createdAt,const QDateTime& lastLogin ,const QString& plainPassword);


    static User* createUser(int id, const QString& username, const QString& email,
                            const QString& password, UserRole role);

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
    QVector<QString> getFavouriteGenre() const;
    void setFavouriteGenre(const QVector<QString> &newFavouriteGenre);
    QDateTime getUpdatedAt() const;
    void setUpdatedAt(const QDateTime &newUpdatedAt);
    bool setPassword(const QString& password);
    bool isAdmin()const;
    bool isPublisher()const;

    AccountStatus getStatus() const;
    void setStatus(AccountStatus newStatus);
    UserRole getRole() const;
    void setRole(UserRole newRole);


};

#endif // USER_H
