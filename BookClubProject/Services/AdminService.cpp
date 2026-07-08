// AdminService.cpp
#include "AdminService.h"


AdminService::AdminService(QObject *parent): QObject(parent)
{

}

QMap<QString, QVariant> AdminService::getSystemStats() const
{
    QMap<QString, QVariant> stats;

    // ===== User Statistics =====
    QVector<User*> allUsers = m_userService->getAllUsers();
    stats["totalUsers"] = allUsers.size();


    // تعداد کاربران بر اساس نقش
    int regularUsers = 0;
    int publishers = 0;
    int admins = 0;
    int blockedUsers = 0;

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
    QVector<QSharedPointer<Book>> allBooks = m_bookService->getAllBooks();
    stats["totalBooks"] = allBooks.size();

    int activeBooks = 0;
    int deactivatedBooks = 0;
    double totalRevenue = 0.0;
    int totalSales = 0;

    for (QSharedPointer<Book> book : allBooks) {
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
    QVector<Review*> allReviews = m_reviewService->getAllReviews();
    stats["totalReviews"] = allReviews.size();

    // ===== Purchase Statistics =====
    QVector<Purchase*> allPurchases = m_purchaseService->getAllPurchases();
    stats["totalPurchases"] = allPurchases.size();

    // ===== System Health =====
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