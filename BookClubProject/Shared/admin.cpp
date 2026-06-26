// admin.cpp
#include "admin.h"






Admin::Admin()
    : User() , adminLevel(AdminLevel::Moderator)
{
    setRole(UserRole::Admin);
}

Admin::Admin(int id, const QString& username, const QString& email, const QString& password)
    : User(id, username, email, password)
    , adminLevel(AdminLevel::Moderator) {
    setRole(UserRole::Admin);
}
/*

Admin::Admin(int id, const QString& username, const QString& email,
             const QString& passwordHash, const QString& salt)
    : User(id, username, email, passwordHash)
    , adminLevel(AdminLevel::Moderator) {
    setRole(UserRole::Admin);
}
*/
Admin::Admin(int id, const QString& fullName, const QString& username, const QString& email,
             UserRole role, AccountStatus status,
             const QDateTime& createdAt, const QDateTime& lastLogin,
             const QString& passwordHash, const QVector<QString>& favouriteGenre,
             const QDateTime& updatedAt, AdminLevel adminLevel , QString  salt)
    : User(id, fullName, username, email, role, status,
           createdAt, lastLogin, passwordHash, favouriteGenre, updatedAt ,salt)  // ← فراخوانی Constructor پایه
    , adminLevel(adminLevel)
    , lastAction(QDateTime::currentDateTime()) {

    // Admin-specific initialization
    setRole(UserRole::Admin);
}




AdminLevel Admin:: getAdminLevel() const { return adminLevel; }
QDateTime Admin:: getLastAction() const { return lastAction; }


void Admin:: setAdminLevel(AdminLevel level) { adminLevel = level; }
void Admin:: setLastAction(const QDateTime& action) { lastAction = action; }


QString Admin:: getAdminLevelString() const {
    switch(adminLevel) {
    case AdminLevel::SuperAdmin: return "SuperAdmin";
    case AdminLevel::Admin:      return "Admin";
    case AdminLevel::Moderator:  return "Moderator";
    default: return "Unknown";
    }
}


bool Admin:: isSuperAdmin() const { return adminLevel == AdminLevel::SuperAdmin; }

bool Admin:: isFullAdmin() const {
    return adminLevel == AdminLevel::SuperAdmin ||
           adminLevel == AdminLevel::Admin;
}

bool Admin:: isModerator() const {
    return adminLevel == AdminLevel::Moderator;
}