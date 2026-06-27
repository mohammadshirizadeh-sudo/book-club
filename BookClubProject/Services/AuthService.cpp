// authservice.cpp
#include "AuthService.h"
#include "../Shared/PasswordValidator.h"
#include "../Shared/PasswordHelper.h"
#include <QDebug>
#include "../Shared/UserFactory.h"

AuthService::AuthService(UserRepository* repo)
    : userRepo(repo) {
}

AuthService::~AuthService() {
    // Repository will be deleted elsewhere
}

User* AuthService::registerUser(const QString& username, const QString& email, const QString& password , UserRole role) {
    // 1. Validate username
    if (username.isEmpty() || username.length() < 3) {
        qDebug() << "Username must be at least 3 characters";
        return nullptr;
    }

    if (userRepo->isUsernameTaken(username)) {
        qDebug() << "Username" << username << "is already taken!";
        return nullptr;
    }


    // 2. Validate email
    ValidationResult emailResult = EmailValidator::isValid(email);
    if (!emailResult.isValid) {
        qDebug() << "Invalid email:" << emailResult.errorMessage;
        return nullptr;
    }

    if (userRepo->isEmailTaken(email)) {
        qDebug() << "Email" << email << "is already registered!";
        return nullptr;
    }

    // 3. Validate password
    ValidationResult PassResult = EmailValidator::isValid(email);
    if (!PassResult.isValid) {
        qDebug() << "Invalid password:" << PassResult.errorMessage;
        return nullptr;
    }

    // 4. Create user

    int newId = userRepo->getNextId();
    User * newUser = UserFactory::createUser(newId , username , email,  password , role);


    // 5. Set password (hash it)


    // 6. Save to repository
    if (!userRepo->addUser(newUser)) {
        delete newUser;
        return nullptr;
    }

    qDebug() << "User registered successfully:" << username;
    return newUser;
}

User* AuthService::login(const QString& usernameOrEmail, const QString& password) {
    // 1. Try to find user by username or email
    User* user = userRepo->findByUsername(usernameOrEmail);
    if (!user) {
        user = userRepo->findByEmail(usernameOrEmail);
    }

    if (!user) {
        qDebug() << "User not found:" << usernameOrEmail;
        return nullptr;
    }

    // 2. Check if user is blocked
    if (user->isBlocked()) {
        qDebug() << "User is blocked:" << user->getUsername();
        return nullptr;
    }

    // 3. Verify password
    if (!user->checkPassword(password)) {
        qDebug() << "Invalid password for user:" << user->getUsername();
        return nullptr;
    }

    // 4. Login successful
    currentUserId = user->getId();
    user->setLastLogin(QDateTime::currentDateTime());
    userRepo->updateUser(user);

    qDebug() << "User logged in:" << user->getUsername();
    return user;
}

bool AuthService::logout() {
    if (currentUserId == -1) {
        qDebug() << "No user is logged in";
        return false;
    }

    currentUserId = -1;
    qDebug() << "User logged out";
    return true;
}

User* AuthService::getCurrentUser() const {
    if (currentUserId == -1) {
        return nullptr;
    }
    return userRepo->findById(currentUserId);
}

bool AuthService::isUsernameAvailable(const QString& username) const {
    return !userRepo->isUsernameTaken(username);
}

