// authservice.h
#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "../Shared/User.h"
#include "../Repositories/UserRepository.h"

class AuthService {
private:
    UserRepository* userRepo;
    int currentUserId = -1;

public:
    AuthService(UserRepository* repo);
    ~AuthService();



    User* registerUser(const QString& username, const QString& email, const QString& password , UserRole role);


    User* login(const QString& usernameOrEmail, const QString& password);

    bool logout();
    User* getCurrentUser() const;
    bool isUsernameAvailable(const QString& username) const;

private:
    User* authenticate(const QString& username, const QString& password);
};

#endif // AUTHSERVICE_H