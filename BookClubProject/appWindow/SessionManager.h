// SessionManager.h
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>

class SessionManager
{
public:
    static SessionManager* instance();

    void setCurrentUser(int userId, const QString& username, const QString& role);
    int getUserId() const { return m_userId; }
    QString getUsername() const { return m_username; }
    QString getRole() const { return m_role; }
    bool isLoggedIn() const { return m_userId > 0; }
    bool isAdmin() const { return m_role == "Admin"; }
    bool isPublisher() const { return m_role == "Publisher"; }
    void clear();

private:
    SessionManager() = default;
    static SessionManager* m_instance;

    int m_userId = 0;
    QString m_username = "";
    QString m_role = "";
};

#endif // SESSIONMANAGER_H