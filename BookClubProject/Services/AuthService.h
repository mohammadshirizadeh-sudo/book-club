// authservice.h
#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "../Shared/User.h"
#include "../Repositories/UserRepository.h"

class AuthService : public QObject {
    Q_OBJECT
private:
    UserRepository* userRepo;
    int currentUserId = -1;

public:
    AuthService(UserRepository* repo , QObject* parent = nullptr);
    ~AuthService();



    User* registerUser(const QString& username, const QString& email, const QString& password , UserRole role);


    User* login(const QString& usernameOrEmail, const QString& password);

    bool logout();
    User* getCurrentUser() const;
    bool isUsernameAvailable(const QString& username) const;




    bool requestPasswordReset(const QString& email);

    bool resetPasswordWithToken(const QString& token, const QString& newPassword);

};

#endif // AUTHSERVICE_H