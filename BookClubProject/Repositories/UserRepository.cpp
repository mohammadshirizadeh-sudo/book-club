// userrepository.cpp
#include "UserRepository.h"
#include <QDebug>

UserRepository::UserRepository() {
    // TODO: Load from file in Phase 3
}

UserRepository::~UserRepository() {
    // Clean up all users
    qDeleteAll(usersById);
}

bool UserRepository::addUser(User* user) {
    if (!user) return false;

    int id = user->getId();

    if (usersById.contains(id)) {
        qWarning() << "User with ID" << id << "already exists!";
        return false;
    }


    if (isUsernameTaken(user->getUsername())) {
        qWarning() << "Username" << user->getUsername() << "is already taken!";
        return false;
    }


    if (isEmailTaken(user->getEmail())) {
        qWarning() << "Email" << user->getEmail() << "is already taken!";
        return false;
    }


    usersById[id] = user;
    usersByUsername[user->getUsername()] = user;
    usersByEmail[user->getEmail()] = user;

    return true;
}

User* UserRepository::findById(int id) const {
    return usersById.value(id, nullptr);
}

User* UserRepository::findByUsername(const QString& username) const {
    return usersByUsername.value(username, nullptr);
}

User* UserRepository::findByEmail(const QString& email) const {
    return usersByEmail.value(email, nullptr);
}

QVector<User*> UserRepository::getAllUsers() const {
    return usersById.values().toVector();
}

bool UserRepository::updateUser(User* user) {
    if (!user) return false;

    int id = user->getId();
    if (!usersById.contains(id)) {
        qWarning() << "User with ID" << id << "not found!";
        return false;
    }

    // Update in all maps
    usersById[id] = user;
    usersByUsername[user->getUsername()] = user;
    usersByEmail[user->getEmail()] = user;


    return true;
}

bool UserRepository::deleteUser(int userId) {
    User* user = usersById.value(userId, nullptr);
    if (!user) {
        qWarning() << "User with ID" << userId << "not found!";
        return false;
    }

    // Remove from all maps
    usersById.remove(userId);
    usersByUsername.remove(user->getUsername());
    usersByEmail.remove(user->getEmail());

    delete user;  // Free memory
    return true;
}

bool UserRepository::isUsernameTaken(const QString& username) const {
    return usersByUsername.contains(username);
}

bool UserRepository::isEmailTaken(const QString& email) const {
    return usersByEmail.contains(email);
}

void UserRepository::resetNextId() {
    int maxId = 1000;
    for (int id : usersById.keys()) {
        if (id > maxId) {
            maxId = id;
        }
    }
    nextId = maxId + 1;
    qDebug() << "nextId reset to:" << nextId;
}



//these are for files don't forget
/*
bool UserRepository::loadFromFile(const QString& filename) {
    // TODO: Implement in Phase 3 (JSON or SQL)
    return false;
}

bool UserRepository::saveToFile(const QString& filename) const {
    // TODO: Implement in Phase 3 (JSON or SQL)
    return false;
}
*/