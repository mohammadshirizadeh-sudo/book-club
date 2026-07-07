// DatabaseManager.h
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QMutex>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    ~DatabaseManager();

    bool openDatabase(const QString& path);
    void closeDatabase();
    bool isOpen() const;
    QSqlDatabase getDatabase() const { return m_database; }

    // ===== Execute Queries =====
    bool executeQuery(const QString& query);
    bool executeQuery(const QString& query, const QVariantMap& params);
    QSqlQuery executeSelect(const QString& query);
    QSqlQuery executeSelect(const QString& query, const QVariantMap& params);

    // ===== Helpers =====
    QString getLastError() const;
    bool transaction();
    bool commit();
    bool rollback();

    static void shutdown();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    static DatabaseManager* m_instance;

    QMutex m_mutex;

    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_isOpen;

    void bindValues(QSqlQuery& query, const QVariantMap& params);

};

#endif // DATABASEMANAGER_H