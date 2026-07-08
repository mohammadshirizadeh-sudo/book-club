// authservice.cpp
#include "AuthService.h"
#include "../Shared/PasswordValidator.h"
#include "../Shared/PasswordHelper.h"
#include <QDebug>
#include "../Shared/UserFactory.h"

AuthService::AuthService(UserRepository* repo , QObject* parent)
    : userRepo(repo) ,QObject(parent) {
}

AuthService::~AuthService() {
    // Repository will be deleted elsewhere
}
/*
ValidationResult AuthService::registerUser(const QString& username, const QString& email, const QString& password , UserRole role) {
    // 1. Validate username
    if (username.isEmpty() || username.length() < 3) {
        return ValidationResult::failure("Username must be at least 3 characters");
    }

    if (userRepo->isUsernameTaken(username)) {
         return ValidationResult::failure("is already taken!");

    }


    // 2. Validate email
    ValidationResult emailResult = EmailValidator::isValid(email);
    if (!emailResult.isValid) {
         return ValidationResult::failure(emailResult.errorMessage);
    }

    if (userRepo->isEmailTaken(email)) {
         return ValidationResult::failure("This Email is already registered!");
    }

    // 3. Validate password
    ValidationResult PassResult = EmailValidator::isValid(email);
    if (!PassResult.isValid) {
        return ValidationResult::failure(PassResult.errorMessage);
    }

    // 4. Create user

    int newId = userRepo->getNextId();
    User * newUser = UserFactory::createUser(newId , username , email,  password , role);


    // 5. Set password (hash it)


    // 6. Save to repository
    if (!userRepo->addUser(newUser)) {
        delete newUser;
        return ValidationResult::failure("");
    }

    qDebug() << "User registered successfully:" << username;
}

*/

ValidationResult AuthService::registerUser(const QString& username, const QString& email,
                                           const QString& password, UserRole role)
{
    // 1. Validate username
    if (username.isEmpty() || username.length() < 3) {
        return ValidationResult::failure("Username must be at least 3 characters");
    }

    if (userRepo->isUsernameTaken(username)) {
        return ValidationResult::failure("Username is already taken!");
    }

    // 2. Validate email
    ValidationResult emailResult = EmailValidator::isValid(email);
    if (!emailResult.isValid) {
        return emailResult;
    }

    if (userRepo->isEmailTaken(email)) {
        return ValidationResult::failure("This email is already registered!");
    }

    ValidationResult passResult = PasswordValidator::isValid(password);
    if (!passResult.isValid) {
        return passResult;
    }

    // 4. Create user
    int newId = userRepo->getNextId();
    User* newUser = UserFactory::createUser(newId, username, email, password, role);
    if (!newUser) {
        return ValidationResult::failure("Failed to create user");
    }

    // 5. Save to repository
    if (!userRepo->addUser(newUser)) {
        delete newUser;
        return ValidationResult::failure("Failed to save user to database");
    }

    qDebug() << "✅ User registered successfully:" << username;

    return ValidationResult::success();
}
/*
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
    userRepo->updateUser(user , user->getUsername(), user->getEmail());

    qDebug() << "User logged in:" << user->getUsername();
    return user;
}*/
// AuthService.cpp
ValidationResult AuthService::login(const QString& usernameOrEmail, const QString& password)
{
    // 1. Try to find user by username or email
    User* user = userRepo->findByUsername(usernameOrEmail);
    if (!user) {
        user = userRepo->findByEmail(usernameOrEmail);
    }

    if (!user) {
        return ValidationResult::failure("User not found");
    }

    // 2. Check if user is blocked
    if (user->isBlocked()) {
        return ValidationResult::failure("Your account has been blocked. Please contact support.");
    }

    // 3. Verify password
    if (!user->checkPassword(password)) {
        return ValidationResult::failure("Invalid password");
    }

    // 4. Login successful
    currentUserId = user->getId();
    user->setLastLogin(QDateTime::currentDateTime());
    userRepo->updateUser(user, user->getUsername(), user->getEmail());

    qDebug() << "✅ User logged in:" << user->getUsername();

    return ValidationResult::success();
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





ValidationResult AuthService::requestPasswordReset(const QString& email)
{

    ValidationResult result = EmailValidator::isValid(email);
    if (!result.isValid) {
        return ValidationResult::failure(result.errorMessage);
    }

    // 2. Find user by email
    User* user = userRepo->findByEmail(email);
    if (!user) {
        return ValidationResult::failure("❌ No user found with email:");
    }

    // 3. Generate reset token
    QString token = user->generateResetToken();
    userRepo->updateUser(user , user->getUsername(), user->getEmail());

    qDebug() << "🔑 Password reset token for" << user->getUsername() << ":" << token;
    qDebug() << "   (expires in 1 hour)";


    return ValidationResult::success();
}

ValidationResult AuthService::resetPasswordWithToken(const QString& token, const QString& newPassword)
{
    // 1. Find user by token (search through all users)
    User* user = nullptr;
    for (User* u : userRepo->getAllUsers()) {
        if (u->getPasswordResetToken() == token) {
            user = u;
            break;
        }
    }

    if (!user) {
        return ValidationResult::failure("❌ Invalid reset token");
    }

    // 2. Reset password with token
    if (!user->resetPasswordWithToken(token, newPassword)) {
        return ValidationResult::failure("❌ Failed to reset password");
    }

    // 3. Save changes
    userRepo->updateUser(user , user->getUsername(), user->getEmail());

    qDebug() << "✅ Password reset successfully for user:" << user->getUsername();
    return ValidationResult::success();
}

User* AuthService::getUserByUsername(const QString& username) const
{
    if (username.isEmpty()) {
        qWarning() << "❌ Username is empty!";
        return nullptr;
    }

    return userRepo->findByUsername(username);
}

