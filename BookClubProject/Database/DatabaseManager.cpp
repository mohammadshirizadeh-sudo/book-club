// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <QFile>
#include <QDir>
#include <QThread>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    static DatabaseManager* inst = new DatabaseManager();
    m_instance = inst;
    return inst;
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
    QSqlDatabase db = connectionForCurrentThread();
    if (!db.isOpen()) {
        qWarning() << "Database is not open for this thread!";
        return false;
    }

    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        qCritical() << "❌ Query failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
        return false;
    }

    return true;
}

bool DatabaseManager::executeQuery(const QString& query, const QVariantMap& params)
{
    QSqlDatabase db = connectionForCurrentThread();
    if (!db.isOpen()) {
        qWarning() << "Database is not open for this thread!";
        return false;
    }

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);
    bindValues(sqlQuery, params);

    if (!sqlQuery.exec()) {
        qCritical() << "❌ Query failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
        return false;
    }

    return true;
}

QSqlQuery DatabaseManager::executeSelect(const QString& query)
{
    QSqlDatabase db = connectionForCurrentThread();
    if (!db.isOpen()) {
        qWarning() << "Database is not open for this thread!";
        return QSqlQuery();
    }

    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        qCritical() << "❌ Select failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
    }
    qDebug()<<"i'm in excute select";
    return sqlQuery;
}


QSqlQuery DatabaseManager::executeSelect(const QString& query, const QVariantMap& params)
{
    QSqlDatabase db = connectionForCurrentThread();
    if (!db.isOpen()) {
        qWarning() << "Database is not open for this thread!";
        return QSqlQuery();
    }

    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);
    bindValues(sqlQuery, params);

    if (!sqlQuery.exec()) {
        qCritical() << "❌ Select failed:" << sqlQuery.lastError().text();
        qCritical() << "   Query:" << query;
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


QSqlDatabase DatabaseManager::connectionForCurrentThread()
{

    const QString connName = QString("conn_%1")
                                 .arg(reinterpret_cast<quintptr>(QThread::currentThread()));

    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase dp = QSqlDatabase::database(connName);
        if (dp.isOpen()) return dp;
        qWarning() << "Connection" << connName << "exists but is not open, recreating...";
        dp.close();
        QSqlDatabase::removeDatabase(connName);

    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(m_databasePath);

    if (!db.open()) {
        qCritical() << "Failed to open database for thread:" << connName;
        return QSqlDatabase();
    }

    QSqlQuery(db).exec("PRAGMA foreign_keys = ON;");
    QSqlQuery(db).exec("PRAGMA busy_timeout = 5000;");

    qDebug() << "✅ Database connection created for thread:" << connName;
    return db;
}