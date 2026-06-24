// admin.h
#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"
#include <QDateTime>


enum class AdminLevel {
    SuperAdmin,
    Admin,
    Moderator
};

class Admin : public User {
private:
    AdminLevel adminLevel;
    QDateTime lastAction;

public:

    Admin();
    Admin(int id, const QString& username, const QString& email, const QString& password);
    Admin(int id, const QString& username, const QString& email,
          const QString& passwordHash, const QString& salt);


    AdminLevel getAdminLevel()const;
    QDateTime getLastAction()const;


    void setAdminLevel(AdminLevel level);
    void setLastAction(const QDateTime& action);


    QString getAdminLevelString() const;


    bool isSuperAdmin() const;

    bool isFullAdmin() const;

    bool isModerator() const;
};

#endif // ADMIN_H