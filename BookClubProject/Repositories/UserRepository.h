// userrepository.h
#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QVector>
#include <QMap>
#include"../Shared/User.h"



class UserRepository {
private:
    QMap<int, User*> usersById;        // Fast lookup by ID
    QMap<QString, User*> usersByUsername; // Fast lookup by username
    QMap<QString, User*> usersByEmail;    // Fast lookup by email
    int nextId = 1000;  // Auto-increment ID

public:
    UserRepository();
    ~UserRepository();


    // ===== CRUD Operations =====


    bool addUser(User* user);

    User* findById(int id) const;

    User* findByUsername(const QString& username) const;

    User* findByEmail(const QString& email) const;

    QVector<User*> getAllUsers() const;
    bool updateUser(User* user);

    bool deleteUser(int userId);

    bool isUsernameTaken(const QString& username) const;

    bool isEmailTaken(const QString& email) const;

    int getNextId() { return nextId++; }

    bool loadFromFile(const QString& filename);

    bool saveToFile(const QString& filename) const;
};

#endif