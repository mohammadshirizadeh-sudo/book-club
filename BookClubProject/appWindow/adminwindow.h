#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QVariantMap>
#include <QTableWidget>
#include <QTimer>
#include <QString>
#include <QDateTime>
#include <functional>

#include "../Network-Manger/NetworkManager.h"
#include "../Server/Response.h"
#include "../Services/AccessLog.h"
#include "../Server/Request.h"
#include "../Users/BookDetailDialog.h"
#include "../Users/UserDetailDialog.h"

struct ActivityLogEntry {
    QDateTime timestamp;
    QString adminName;
    QString action;
    QString targetUser;
};

namespace Ui { class AdminWindow; }

struct UserData {
    int id = 0;
    QString fullName;
    QString email;
    QString type;
    QString status;
    QString registeredDate;
    QString lastLoginDate;
};

struct AdminBookData {
    int id = 0;
    QString title;
    QString author;
    int publisherId = -1;
    QString publisherName = "unknown";
    double price = 0.0;
    QString status;
    int salesCount = 0;
    double averageRating = 0.0;
};

struct AdminReviewData {
    int id = 0;
    int bookId = 0;
    int reviewerId = 0;
    int rating = 0;
    QString reviewText;
    QString status;
    bool isFlagged = false;
    QString date;
};

struct DashboardStats {
    int totalUsers = 0;
    int totalPublishers = 0;
    int totalBooks = 0;
    double totalRevenue = 0.0;
};

struct BlockedUserInfo {
    int userId = 0;
    QString name;
    QString email;
    QString blockedByAdmin;
    QString reason;
    QString blockedDate;
};

class AdminWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AdminWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~AdminWindow();

    void initializeFromServer();
    void setAdminInfo();
    QString formatTimeAgo(const QDateTime &dt) const;

signals:
    void signOutRequested();
    void navigateToBookDetails(int bookId);
    void showUserDetailsRequested(int userId);

    void editWindow();
    void viewBookReviewsRequested(int selectedBookId);

private slots:
    void handleDashboardButtonClicked();
    void handleUserManageButtonClicked();
    void handleAccessControlButtonClicked();
    void handleContentManageButtonClicked();
    void handleReviewsMonitorButtonClicked();
    void handleSystemLogsButtonClicked();
    void handleServerStatusButtonClicked();
    void handleSignOutButtonClicked();

    void handleRefreshDashboardClicked();
    void handleExportReportClicked();
    void handleBackupClicked();
    void handleClearCacheClicked();
    void handleBroadcastMsgClicked();
    void handleCheckServerClicked();
    void handleRestartServerClicked();

    void handleAllUsersRadioToggled(bool checked);
    void handleRegularUsersRadioToggled(bool checked);
    void handlePublishersRadioToggled(bool checked);
    void handleBlockedUsersRadioToggled(bool checked);
    void handleUserSearchTextChanged(const QString &text);
    void handleRefreshUsersClicked();
    void handleExportUsersClicked();
    void handleUsersTableCellClicked(int row, int column);
    void handleViewUserDetailsClicked();
    void handleBlockUserClicked();
    void handleUnblockUserClicked();
    void handleDeleteUserClicked();
    void handleToggleActiveClicked();

    void handleBookSearchTextChanged(const QString &text);
    void handlePublisherFilterChanged(int index);
    void handleStatusFilterChanged(int index);
    void handleRefreshBooksClicked();
    void handleAdminBooksTableCellClicked(int row, int column);
    void handleViewBookDetailsClicked();
    void handleFlagBookClicked();
    void handleDeleteBookClicked();
    void handleDeactivateBookClicked();

    void handleReviewStatusFilterChanged(int index);
    void handleReviewRatingFilterChanged(int index);
    void handleReviewSearchTextChanged(const QString &text);
    void handleRefreshReviewsClicked();
    void handleReviewsMonitorTableCellClicked(int row, int column);
    void handleApproveReviewClicked();
    void handleRejectReviewClicked();
    void handleFlagReviewClicked();
    void handleDeleteReviewClicked();

    void updateServerStatusClock();
    void handleRefreshStatsClicked();
    void handleClearLogsClicked();

    void onResponseReceived(const Response &response);

    void on_signOutButton_clicked();

private:
    Ui::AdminWindow *ui;
    NetworkManager *m_networkManager;

    int m_adminId = 0;
    QString m_adminName;

    int m_selectedUserId = -1;
    UserData m_selectedUserData;
    int m_selectedBookId = -1;
    AdminBookData m_selectedBookData;
    int m_selectedReviewId = -1;
    AdminReviewData m_selectedReviewData;

    QString m_userFilterType = "all";
    QString m_userSearchText;
    QString m_bookSearchText;
    QString m_bookStatusFilter;
    int m_publisherFilterId = -1;
    QString m_reviewStatusFilter;
    int m_reviewRatingFilter = 0;
    QString m_reviewSearchText;
    int m_reviewsBookIdFilter = -1;

    bool m_connectionsSetup = false;
    qint64 m_pendingBookDetailsRequestId = -1;

    QTimer* m_serverStatusRefreshTimer = nullptr;
    QTimer* m_serverStatusClockTimer = nullptr;

    void setupConnections();
    void setupUIInitialState();
    void switchToPage(int pageIndex);

    void requestDashboardStats();
    void requestRecentActivity();
    void requestSystemAlerts();
    void requestServerStatus();
    void requestDatabaseStatus();
    void requestServerResourceUsage();
    void requestConnectedClients();
    void requestTrafficStats();
    void refreshServerStatusPage();
    void requestUsersList();
    void requestBlockedUsers();
    void requestAccessLog();
    void requestBooksList();
    void requestReviewsList(int limit = -1);

    void handleDashboardStats(const Response &r);
    void handleRecentActivity(const Response &r);
    void handleSystemAlerts(const Response &r);
    void handleServerStatus(const Response &r);
    void handleDatabaseStatus(const Response &r);
    void handleResourceUsage(const Response &r);
    void handleConnectedClients(const Response &r);
    void handleTrafficStats(const Response &r);
    void handleUsersList(const Response &r);
    void handleBlockedUsers(const Response &r);
    void handleAccessLog(const Response &r);
    void handleBooksList(const Response &r);
    void handleReviewsList(const Response &r);
    void handleGenericActionResult(const Response &r, const QString &successMsg, std::function<void()> onSuccess);

    void handleEditBookClicked();
    void handleViewReviewsClicked();
    void logServerEvent(const QString &message, const QString &level = "INFO");

    void populateDashboardStats(const DashboardStats &stats);
    void populateActivityLog(const QList<ActivityLogEntry> &logs);
    void populateSystemAlerts(const QStringList &alerts);
    void populateUsersTable(const QList<UserData> &users);
    void populateBlockedUsersTable(const QList<BlockedUserInfo> &blockedUsers);
    void populateAccessLogTable(const QList<AccessLogEntry> &logs);
    void populateBooksTable(const QList<AdminBookData> &books);
    void populateReviewsTable(const QList<AdminReviewData> &reviews);

    void updateUserSelectionState(int row);
    void updateUserActionButtons();
    void updateBookSelectionState(int row);
    void updateBookActionButtons();
    void updateReviewSelectionState(int row);
    void updateReviewActionButtons();

    void showError(const QString &title, const QString &message);
    void showSuccess(const QString &message);
    void showConfirmation(const QString &title, const QString &message, std::function<void()> onConfirm);
    void clearTable(QTableWidget *table);
    QString formatDateTime(const QString &isoDate) const;
    QString formatTimeAgo(const QString &isoDate) const;
    QString getStarString(int rating) const;
};

#endif // ADMINWINDOW_H