// userrepository.h
#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QVector>
#include <QMap>
#include"../Shared/User.h"
#include "../Database/DatabaseInitializer.h"
#include "../Shared/Genre.h"
#include "../Shared/Publisher.h"
#include "../Shared/admin.h"

#include "../Database/DatabaseManager.h"


class UserRepository : public QObject {
    Q_OBJECT
private:
    QMap<int, User*> usersById;        // Fast lookup by ID
    QMap<QString, User*> usersByUsername; // Fast lookup by username
    QMap<QString, User*> usersByEmail;    // Fast lookup by email
    int nextId = 1000;  // Auto-increment ID


    mutable QMutex m_mutex;

    void addToCache(User* user);
    void removeFromCache(int userId);
    void clearCache();

public:
    UserRepository(QObject* parent = nullptr);
    ~UserRepository();


    // ===== CRUD Operations =====


    bool addUser(User* user);

    User* findById(int id) const;

    User* findByUsername(const QString& username) const;

    User* findByEmail(const QString& email) const;

    QVector<User*> getAllUsers() const;
    bool updateUser(User* user, const QString& oldUsername, const QString& oldEmail);

    bool deleteUser(int userId);

    bool isUsernameTaken(const QString& username) const;

    bool isEmailTaken(const QString& email) const;

    int getNextId() { return nextId++; }


    bool loadAllFromDatabase();
    bool saveToDatabase(User* user);
    bool deleteFromDatabase(int userId);
    void resetNextId();

    static UserRole stringToRole(const QString& roleStr);

    AccountStatus stringToStatus(const QString& statusStr);
    QString roleToString(UserRole role);
    QString statusToString(AccountStatus status);


    QVector<Genre> stringToGenres(const QString& str) const;


    QString genresToString(const QVector<Genre>& genres) const;
    void loadPublisherInfo(Publisher* publisher);

    bool savePublisherInfo(Publisher* publisher);
    void loadAdminInfo(Admin* admin);
    AdminLevel stringToAdminLevel(const QString& str) const;


    bool saveAdminInfo(Admin* admin);


    QString adminLevelToString(AdminLevel level) const;


    QVector<User*> searchUsers(const QString& keyword) const;

};

#endif