// AdminService.cpp
#include "AdminService.h"
#include "../Database/DatabaseManager.h"
#include "../Services/LibraryService.h"
#include <algorithm>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>



AdminService::AdminService(UserService* userService,
                           BookService* bookService,
                           ReviewService* reviewService,
                           PurchaseService* purchaseService,
                           NotificationService* notifService,
                           LibraryService* libraryService,
                           Server* server,
                           QObject* parent)
    : QObject(parent)
    , m_userService(userService)
    , m_bookService(bookService)
    , m_reviewService(reviewService)
    , m_purchaseService(purchaseService)
    , m_notifService(notifService)
    , m_libraryService(libraryService)
    , m_server(server)
{
    loadAccessLogsFromDatabase();
}

QMap<QString, QVariant> AdminService::getSystemStats() const
{
    QMap<QString, QVariant> stats;

    // ===== User Statistics =====
    QVector<User*> allUsers = m_userService->getAllUsers();
    stats["total_users"] = allUsers.size();

    int regularUsers = 0, publishers = 0, admins = 0, blockedUsers = 0;
    for (User* user : allUsers) {
        if (user->isBlocked()) blockedUsers++;
        if (user->isAdmin()) admins++;
        else if (user->isPublisher()) publishers++;
        else regularUsers++;
    }
    stats["regularUsers"] = regularUsers;
    stats["total_publishers"] = publishers;
    stats["admins"] = admins;
    stats["blockedUsers"] = blockedUsers;

    // ===== Book Statistics =====
    // کتاب‌های فعال و غیرفعال (همه کتاب‌ها)
    QVector<QSharedPointer<Book>> allBooks = m_bookService->getBookRepo()->getAllBooks();
    stats["total_books"] = allBooks.size();

    int activeBooks = 0, deactivatedBooks = 0, totalSales = 0;
    double totalRevenue = 0.0;

    for (const QSharedPointer<Book>& book : allBooks) {
        if (book->getIsActive()) activeBooks++;
        else deactivatedBooks++;

        totalRevenue += book->getFinalPrice() * book->getSalesCount();
        totalSales += book->getSalesCount();
    }

    stats["activeBooks"] = activeBooks;
    stats["deactivatedBooks"] = deactivatedBooks;
    stats["total_revenue"] = totalRevenue;
    stats["totalSales"] = totalSales;

    // ===== Review Statistics =====
    stats["totalReviews"] = m_reviewService->getAllReviews().size();

    // ===== Purchase Statistics =====
    stats["totalPurchases"] = m_purchaseService->getAllPurchases().size();

    stats["systemStatus"] = "Healthy";
    return stats;
}
bool AdminService::blockUser(int id , const QString& reason)
{
    return m_userService->blockUser(id, reason);
}

bool AdminService::unblockUser(int id)
{
    return  m_userService->unblockUser(id);
}

bool AdminService::deleteUser(int id)
{
    return m_userService->deleteUser(id);
}

QVector<User *> AdminService::getBlockedUsers() const
{
    return m_userService->getBlockedUsers();
}


QVector<QVariantMap> AdminService::getRecentActivities(int limit) const
{
    struct Activity
    {
        QDateTime time;
        QString adminName;
        QString action;
        QString targetUser;
    };

    QVector<Activity> events;

    for (const auto& p : m_purchaseService->getAllPurchases())
    {
        events.push_back({
            p->getPurchasedAt(),
            "System",
            QString("Purchase #%1 completed — status: %2")
                .arg(p->getPurchaseId())
                .arg(p->getStatusString()),
            QString()
        });
    }

    for (const auto& r : m_reviewService->getAllReviews())
    {
        events.push_back({
            r->getCreatedAt(),
            "System",
            QString("Review posted on book #%1 (rating %2/5)")
                .arg(r->getBookId())
                .arg(r->getRating()),
            QString()
        });
    }

    {
        QMutexLocker locker(&m_logMutex);

        for (const auto& entry : m_accessLog)
        {
            events.push_back({
                entry.timestamp,
                entry.adminName,
                entry.action,
                entry.targetUser
            });
        }
    }

    std::sort(events.begin(), events.end(),
              [](const Activity& a, const Activity& b)
              {
                  return a.time > b.time;
              });

    QVector<QVariantMap> result;

    for (int i = 0; i < events.size() && i < limit; ++i)
    {
        QVariantMap m;

        m["timestamp"] = events[i].time.toString(Qt::ISODate);
        m["adminName"] = events[i].adminName;
        m["action"] = events[i].action;
        m["targetUser"] = events[i].targetUser;

        result.append(m);
    }

    return result;
}



QStringList AdminService::getSystemAlerts() const
{
    QStringList alerts;
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        alerts << "⚠️ Database connection is not open!";
    }
    auto blocked = m_userService->getBlockedUsers();
    if (blocked.size() > 10) {
        alerts << QString("⚠️ %1 users currently blocked").arg(blocked.size());
    }
    auto allBooks = m_bookService->getBookRepo()->getAllBooks();
    int active = 0;
    for (const auto& b : allBooks) {
        if (b->getIsActive()) active++;
    }
    if (!allBooks.isEmpty() && active == 0) {
        alerts << "⚠️ No active books available in the store!";
    }

    if (alerts.isEmpty()) {
        alerts << "✅ All systems operational. No alerts at this time.";
    }

    return alerts;
}

QMap<QString, QVariant> AdminService::getDatabaseStatus() const
{
    QMap<QString, QVariant> status;
    DatabaseManager* db = DatabaseManager::instance();

    status["isOpen"] = db->isOpen();
    if (db->isOpen()) {
        status["status"] = "Connected";
        status["statusMessage"] = "Database connection is active";
        status["health"] = "Healthy";
    } else {
        status["status"] = "Disconnected";
        status["statusMessage"] = "Database connection is not active";
        status["health"] = "Critical";
    }
    return status;
}
// AdminService.cpp
#include "AccessLog.h"
#include <QMutexLocker>

void AdminService::appendAccessLog(const AccessLogEntry& entry)

{
    {
        QMutexLocker locker(&m_logMutex);
        m_accessLog.append(entry);
    }

    QString query = R"(
        INSERT INTO access_log
        (timestamp, admin_name, action, target_user, ip_address, status)
        VALUES
        (:timestamp, :admin_name, :action, :target_user, :ip_address, :status)
    )";

    QVariantMap params;
    params["timestamp"] = entry.timestamp.toString(Qt::ISODate);
    params["admin_name"] = entry.adminName;
    params["action"] = entry.action;
    params["target_user"] = entry.targetUser;
    params["ip_address"] = entry.ipAddress;
    params["status"] = entry.status;

    DatabaseManager::instance()->executeQuery(query, params);

    qDebug() << "📋 Access Log:"
             << entry.adminName
             << "->"
             << entry.action
             << "on"
             << entry.targetUser
             << "("
             << entry.status
             << ")";
}

// =============================================
// ===== getAccessLogs =====
// =============================================

QVector<AccessLogEntry> AdminService::getAccessLogs() const
{
    QMutexLocker locker(&m_logMutex);
    return m_accessLog;
}

// =============================================
// ===== clearAccessLogs =====
// =============================================

void AdminService::clearAccessLogs()
{
    QMutexLocker locker(&m_logMutex);
    m_accessLog.clear();
    qDebug() << "🗑️ Access logs cleared";
}




int AdminService::getOnlineUserCount() const
{
    if (m_server) {
        return m_server->getOnlineUserCount();
    }
    qWarning() << "Server pointer is null!";
    return 0;
}

bool AdminService::isDatabaseConnected() const
{
    DatabaseManager* db = DatabaseManager::instance();
    if (!db) {
        qWarning() << "DatabaseManager instance is null!";
        return false;
    }
    return db->isOpen();
}


QString AdminService::getServerUptimeString() const
{
    if (m_server) {
        return m_server->getUptimeString();
    }
    qWarning() << "Server pointer is null!";
    return "00:00:00";
}





QString AdminService::backupDatabase() const
{
    // 1. دریافت مسیر دیتابیس فعلی
    DatabaseManager* db = DatabaseManager::instance();
    if (!db) {
        qWarning() << "❌ DatabaseManager instance is null!";
        return QString();
    }

    if (!db->isOpen()) {
        qWarning() << "❌ Database is not open!";
        return QString();
    }

    // 2. دریافت نام فایل دیتابیس
    QSqlDatabase sqlDb = db->getDatabase();
    QString dbPath = sqlDb.databaseName();

    if (dbPath.isEmpty()) {
        qWarning() << "❌ Database path is empty!";
        return QString();
    }

    QFileInfo dbFileInfo(dbPath);
    if (!dbFileInfo.exists()) {
        qWarning() << "❌ Database file does not exist:" << dbPath;
        return QString();
    }

    // 3. ساخت مسیر پشتیبان
    QDir backupDir = dbFileInfo.absoluteDir();

    // ایجاد پوشه backups اگر وجود ندارد
    if (!backupDir.exists("backups")) {
        if (!backupDir.mkdir("backups")) {
            qWarning() << "❌ Failed to create backups directory!";
            return QString();
        }
    }

    backupDir.cd("backups");

    // 4. ساخت نام فایل پشتیبان با تاریخ و زمان
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString backupFileName = QString("%1_backup_%2.db")
                                 .arg(dbFileInfo.baseName())
                                 .arg(timestamp);
    QString backupPath = backupDir.absoluteFilePath(backupFileName);

    // 5. کپی فایل
    if (!QFile::copy(dbPath, backupPath)) {
        qWarning() << "❌ Failed to copy database to:" << backupPath;
        return QString();
    }

    // 6. بررسی حجم فایل پشتیبان
    QFileInfo backupInfo(backupPath);
    if (backupInfo.size() != dbFileInfo.size()) {
        qWarning() << "⚠️ Backup size mismatch! Original:"
                   << dbFileInfo.size() << "Backup:" << backupInfo.size();
        // هنوز هم فایل وجود دارد، ولی warning می‌دهیم
    }

    qDebug() << "✅ Database backup created:" << backupPath;
    qDebug() << "   Size:" << (backupInfo.size() / 1024) << "KB";
    qDebug() << "   Location:" << backupPath;

    return backupPath;
}



// AdminService.cpp
void AdminService::scheduleRestart(int delayMs)
{
    if (!m_server) {
        qWarning() << "❌ Server pointer is null! Cannot schedule restart.";
        return;
    }

    m_server->scheduleRestart(delayMs);
}

void AdminService::cancelRestart()
{
    if (!m_server) {
        qWarning() << "❌ Server pointer is null! Cannot cancel restart.";
        return;
    }

    m_server->cancelRestart();
}


// AdminService.cpp
void AdminService::clearCaches()
{
    qDebug() << "🗑️ Clearing all caches...";

    // 1. Clear UserRepository (if accessible)
    if (m_userService) {
        // اگر UserService به UserRepository دسترسی دارد
        // m_userService->clearCaches();
    }

    // 2. Clear BookRepository
    if (m_bookService) {
        BookRepository* bookRepo = m_bookService->getBookRepo();
        if (bookRepo) {
            bookRepo->clearCache();
        }
    }

    // 3. Clear ReviewRepository
    if (m_reviewService) {
        ReviewRepository* reviewRepo = m_reviewService->getReviewRepo();
        if (reviewRepo) {
            reviewRepo->clearCache();
        }
    }

    // 4. Clear PurchaseRepository
    if (m_purchaseService) {
        PurchaseRepository* purchaseRepo = m_purchaseService->getPurchaseRepo();
        if (purchaseRepo) {
            purchaseRepo->clearCache();
        }
    }

    // 5. Clear LibraryRepository
    if (m_libraryService) {
        LibraryRepository* libraryRepo = m_libraryService->getLibraryRepo();
        if (libraryRepo) {
            libraryRepo->clearCache();
        }
    }

    qDebug() << "✅ All caches cleared successfully!";
}



void AdminService::loadAccessLogsFromDatabase()
{
    QSqlQuery q = DatabaseManager::instance()->executeSelect(
        "SELECT timestamp, admin_name, action, target_user, ip_address, status "
        "FROM access_log ORDER BY id"
        );

    QMutexLocker locker(&m_logMutex);
    m_accessLog.clear();

    while (q.next())
    {
        m_accessLog.append(
            AccessLogEntry(
                QDateTime::fromString(
                    q.value("timestamp").toString(),
                    Qt::ISODate
                    ),
                q.value("admin_name").toString(),
                q.value("action").toString(),
                q.value("target_user").toString(),
                q.value("ip_address").toString(),
                q.value("status").toString()
                )
            );
    }

    qDebug() << "✅ Loaded"
             << m_accessLog.size()
             << "access log entries from SQLite";
}








