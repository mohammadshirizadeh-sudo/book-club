// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <QFile>
#include <QDir>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_isOpen(false)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::openDatabase(const QString& path)
{
    if (m_isOpen) {
        closeDatabase();
    }

    m_databasePath = path;

    // Create directory if not exists
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_database.setDatabaseName(path);

    if (!m_database.open()) {
        qCritical() << "❌ Failed to open database:" << m_database.lastError().text();
        m_isOpen = false;
        return false;
    }

    m_isOpen = true;
    qDebug() << "✅ Database opened successfully:" << path;

    QSqlQuery query(m_database);
    if (query.exec("PRAGMA foreign_keys = ON;")) {
        qDebug() << "✅ Foreign keys ENABLED (ON DELETE CASCADE will work)";
    } else {
        qWarning() << "⚠️ Failed to enable foreign keys:" << query.lastError().text();
    }

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_isOpen) {
        m_database.close();
        m_isOpen = false;
        qDebug() << "🔒 Database closed";
    }
}

bool DatabaseManager::isOpen() const
{
    return m_isOpen && m_database.isOpen();
}

bool DatabaseManager::executeQuery(const QString& query)
{
    if (!isOpen()) {
        qWarning() << "⚠️ Database is not open!";
        return false;
    }

    QSqlQuery sqlQuery(m_database);
    if (!sqlQuery.exec(query)) {
        qCritical() << "❌ Query failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
        return false;
    }

    return true;
}

bool DatabaseManager::executeQuery(const QString& query, const QVariantMap& params)
{
    if (!isOpen()) {
        qWarning() << "⚠️ Database is not open!";
        return false;
    }

    QSqlQuery sqlQuery(m_database);
    sqlQuery.prepare(query);
    bindValues(sqlQuery, params);

    if (!sqlQuery.exec()) {
        qCritical() << "❌ Query failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::executeSelect(const QString& query) {
    if (!isOpen()) {
        qWarning() << "⚠️ DatabaseManager::executeSelect: Database is not open!";
        return QSqlQuery();
    }

    QSqlQuery sqlQuery(m_database);
    if (!sqlQuery.exec(query)) {
        qWarning() << "❌ executeSelect failed:" << sqlQuery.lastError().text();
        qWarning() << "   Query:" << query;
    }

    return sqlQuery;
}

QSqlQuery DatabaseManager::executeSelect(const QString& query, const QVariantMap& params) {
    if (!isOpen()) {
        qWarning() << "⚠️ DatabaseManager::executeSelect: Database is not open!";
        return QSqlQuery();
    }

    QSqlQuery sqlQuery(m_database);
    if (!sqlQuery.prepare(query)) {
        qWarning() << "❌ executeSelect: Failed to prepare query!";
        qWarning() << "   Query:" << query;
        qWarning() << "   Error:" << sqlQuery.lastError().text();
        return sqlQuery;
    }
    bindValues(sqlQuery, params);
    if (!sqlQuery.exec()) {
        qWarning() << "❌ executeSelect: Failed to execute query!";
        qWarning() << "   Query:" << query;
        qWarning() << "   Params:" << params;
        qWarning() << "   Error:" << sqlQuery.lastError().text();
    }

    return sqlQuery;
}




//remember you must use it in the main!!!!!
void DatabaseManager::shutdown() {
    if (m_instance) {
        m_instance->closeDatabase();
        delete m_instance;
        m_instance = nullptr;
        qDebug() << "✅ DatabaseManager shutdown complete";
    }
}
void DatabaseManager::bindValues(QSqlQuery& query, const QVariantMap& params)
{
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
}
QString DatabaseManager::getLastError() const
{
    return m_database.lastError().text();
}

bool DatabaseManager::transaction()
{
    if (!isOpen()) return false;
    return m_database.transaction();
}

bool DatabaseManager::commit()
{
    if (!isOpen()) return false;
    return m_database.commit();
}

bool DatabaseManager::rollback()
{
    if (!isOpen()) return false;
    return m_database.rollback();
}