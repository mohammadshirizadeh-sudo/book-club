// SessionManager.cpp
#include "SessionManager.h"

SessionManager* SessionManager::m_instance = nullptr;

SessionManager* SessionManager::instance()
{
    if (!m_instance) {
        m_instance = new SessionManager();
    }
    return m_instance;
}

void SessionManager::setCurrentUser(int userId, const QString& username, const QString& role)
{
    m_userId = userId;
    m_username = username;
    m_role = role;
}

void SessionManager::clear()
{
    m_userId = 0;
    m_username = "";
    m_role = "";
}