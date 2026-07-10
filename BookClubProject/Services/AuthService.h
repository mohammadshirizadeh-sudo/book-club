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



    ValidationResult registerUser(const QString& username, const QString& email, const QString& password , UserRole role);


    ValidationResult login(const QString& usernameOrEmail, const QString& password);

    bool logout();
    User* getCurrentUser() const;
    bool isUsernameAvailable(const QString& username) const;




    ValidationResult requestPasswordReset(const QString& email);

    ValidationResult resetPasswordWithToken(const QString& token, const QString& newPassword);

    User* getUserByUsername(const QString& username) const;

};

#endif // AUTHSERVICE_H