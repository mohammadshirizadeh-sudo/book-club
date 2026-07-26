// AdminService.cpp
#include "AdminService.h"
#include "../Database/DatabaseManager.h"
#include <algorithm>


AdminService::AdminService(UserService* userService,
                           BookService* bookService,
                           ReviewService* reviewService,
                           PurchaseService* purchaseService,
                           NotificationService* notifService,
                           QObject* parent)
    : QObject(parent)
    , m_userService(userService)
    , m_bookService(bookService)
    , m_reviewService(reviewService)
    , m_purchaseService(purchaseService)
    , m_notifService(notifService)
{
}

QMap<QString, QVariant> AdminService::getSystemStats() const
{
    QMap<QString, QVariant> stats;

    // ===== User Statistics =====
    QVector<User*> allUsers = m_userService->getAllUsers();
    stats["totalUsers"] = allUsers.size();

    int regularUsers = 0, publishers = 0, admins = 0, blockedUsers = 0;
    for (User* user : allUsers) {
        if (user->isBlocked()) blockedUsers++;
        if (user->isAdmin()) admins++;
        else if (user->isPublisher()) publishers++;
        else regularUsers++;
    }
    stats["regularUsers"] = regularUsers;
    stats["publishers"] = publishers;
    stats["admins"] = admins;
    stats["blockedUsers"] = blockedUsers;

    // ===== Book Statistics =====
    // کتاب‌های فعال و غیرفعال (همه کتاب‌ها)
    QVector<QSharedPointer<Book>> allBooks = m_bookService->getBookRepo()->getAllBooks();
    stats["totalBooks"] = allBooks.size();

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
    stats["totalRevenue"] = totalRevenue;
    stats["totalSales"] = totalSales;

    // ===== Review Statistics =====
    stats["totalReviews"] = m_reviewService->getAllReviews().size();

    // ===== Purchase Statistics =====
    stats["totalPurchases"] = m_purchaseService->getAllPurchases().size();

    stats["systemStatus"] = "Healthy";
    return stats;
}
bool AdminService::blockUser(int id , QString reason)
{
    return m_userService->blockUser(id);
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


QStringList AdminService::getRecentActivities(int limit) const
{
    struct Activity {
        QDateTime time;
        QString text;
    };
    QVector<Activity> events;
    for (const auto& p : m_purchaseService->getAllPurchases()) {
        events.push_back({
            p->getPurchasedAt(),
            QString("Purchase #%1 completed — status: %2")
                .arg(p->getPurchaseId())
                .arg(p->getStatusString())
        });
    }
    for (const auto& r : m_reviewService->getAllReviews()) {
        events.push_back({
            r->getCreatedAt(),
            QString("Review posted on book #%1 (rating %2/5)")
                .arg(r->getBookId())
                .arg(r->getRating())
        });
    }
    std::sort(events.begin(), events.end(),
              [](const Activity& a, const Activity& b) {
                  return a.time > b.time;
              });

    QStringList activities;
    for (int i = 0; i < events.size() && i < limit; ++i) {
        activities << events[i].text;
    }

    return activities;
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