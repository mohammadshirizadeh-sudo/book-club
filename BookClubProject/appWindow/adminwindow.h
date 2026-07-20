#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>

namespace Ui {
class AdminWindow;
}

// ==================== DATA STRUCTURES ====================

// User data from server
struct UserData {
    int id = -1;
    QString username;
    QString email;
    QString fullName;
    QString type;  // "regular" or "publisher"
    QString status;  // "active", "inactive", "blocked"
    QString registeredDate;
    QString lastLoginDate;
    int booksCount = 0;
    double totalSpent = 0.0;

    static UserData fromJson(const QJsonObject &json);
};

// Book data for admin view
struct AdminBookData {
    int id = -1;
    QString title;
    QString author;
    QString publisherName;
    int publisherId = -1;
    double price = 0.0;
    QString status;  // "active", "inactive", "flagged"
    int salesCount = 0;
    double averageRating = 0.0;
    int reviewsCount = 0;
    QString publishDate;
    QString coverUrl;

    static AdminBookData fromJson(const QJsonObject &json);
};

// Review data for monitoring
struct AdminReviewData {
    int id = -1;
    int bookId = -1;
    QString bookTitle;
    int reviewerId = -1;
    QString reviewerName;
    int rating = 0;
    QString reviewText;
    QString date;
    QString status;  // "pending", "approved", "rejected", "flagged"
    bool isFlagged = false;

    static AdminReviewData fromJson(const QJsonObject &json);
};

// Dashboard statistics
struct DashboardStats {
    int totalUsers = 0;
    int totalPublishers = 0;
    int totalBooks = 0;
    double totalRevenue = 0.0;
    int onlineUsers = 0;
    QString serverUptime;
    bool dbConnected = false;
    bool serverOnline = false;
};

// Activity log entry
struct ActivityLogEntry {
    QString timestamp;
    QString user;
    QString action;
    QString details;

    static ActivityLogEntry fromJson(const QJsonObject &json);
};

// Access control log entry
struct AccessLogEntry {
    QString timestamp;
    QString adminName;
    QString action;
    QString targetUser;
    QString ipAddress;
    QString status;

    static AccessLogEntry fromJson(const QJsonObject &json);
};

// Blocked user info
struct BlockedUserInfo {
    int userId = -1;
    QString name;
    QString email;
    QString blockedByAdmin;
    QString reason;
    QString blockedDate;

    static BlockedUserInfo fromJson(const QJsonObject &json);
};

// ==================== ADMIN WINDOW CLASS ====================

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

    // === MAIN ENTRY POINT ===
    void initializeFromServer();  // Load all initial data from server

    // Configuration setters
    void setApiBaseUrl(const QString &baseUrl);
    void setAdminToken(const QString &token);
    void setAdminInfo(int adminId, const QString &adminName);

signals:
    void signOutRequested();
    void navigateToPublisherPanel(int publisherId);
    void navigateToBookDetails(int bookId);
    void showUserDetailsRequested(int userId);

private slots:
    // === NAVIGATION SLOTS ===
    void on_dashboardButton_clicked();
    void on_userManageButton_clicked();
    void on_accessControlButton_clicked();
    void on_contentManageButton_clicked();
    void on_reviewsMonitorButton_clicked();
    void on_systemLogsButton_clicked();
    void on_serverStatusButton_clicked();
    void on_notifButton_clicked();
    void on_signOutButton_clicked();

    // === DASHBOARD SLOTS ===
    void on_refreshDashboardBtn_clicked();
    void on_exportReportBtn_clicked();
    void on_backupBtn_clicked();
    void on_clearCacheBtn_clicked();
    void on_broadcastMsgBtn_clicked();
    void on_checkServerBtn_clicked();
    void on_restartServerBtn_clicked();

    // === USER MANAGEMENT SLOTS ===
    void on_allUsersRadio_toggled(bool checked);
    void on_regularUsersRadio_toggled(bool checked);
    void on_publishersRadio_toggled(bool checked);
    void on_blockedUsersRadio_toggled(bool checked);
    void on_userSearchLineEdit_textChanged(const QString &text);
    void on_refreshUsersBtn_clicked();
    void on_exportUsersBtn_clicked();
    void on_usersTable_cellClicked(int row, int column);
    void on_viewUserDetailsBtn_clicked();
    void on_blockUserBtn_clicked();
    void on_unblockUserBtn_clicked();
    void on_deleteUserBtn_clicked();
    void on_toggleActiveBtn_clicked();
    void on_loginAsUserBtn_clicked();

    // === ACCESS CONTROL SLOTS ===
    void on_blockedUsersTable_cellClicked(int row, int column);

    // === CONTENT MANAGEMENT SLOTS ===
    void on_bookSearchLineEdit_textChanged(const QString &text);
    void on_publisherFilterCombo_currentIndexChanged(int index);
    void on_statusFilterCombo_currentIndexChanged(int index);
    void on_refreshBooksBtn_clicked();
    void on_adminBooksTable_cellClicked(int row, int column);
    void on_viewBookDetailsBtn_clicked();
    void on_editBookBtn_clicked();
    void on_flagBookBtn_clicked();
    void on_deleteBookBtn_clicked();
    void on_viewReviewsBtn_clicked();
    void on_deactivateBookBtn_clicked();

    // === REVIEWS MONITORING SLOTS ===
    void on_reviewStatusFilter_currentIndexChanged(int index);
    void on_reviewRatingFilter_currentIndexChanged(int index);
    void on_reviewSearchEdit_textChanged(const QString &text);
    void on_refreshReviewsBtn_clicked();
    void on_reviewsMonitorTable_cellClicked(int row, int column);
    void on_approveReviewBtn_clicked();
    void on_rejectReviewBtn_clicked();
    void on_flagReviewBtn_clicked();
    void on_deleteReviewBtn_clicked();
    void on_replyToReviewerBtn_clicked();
    void on_viewReviewerProfileBtn_clicked();

    // === NETWORK RESPONSE HANDLERS ===
    void onDashboardStatsReceived(QNetworkReply *reply);
    void onActivityLogReceived(QNetworkReply *reply);
    void onAlertsReceived(QNetworkReply *reply);
    void onServerStatusReceived(QNetworkReply *reply);
    void onUsersListReceived(QNetworkReply *reply);
    void onBlockUserResponse(QNetworkReply *reply);
    void onUnblockUserResponse(QNetworkReply *reply);
    void onDeleteUserResponse(QNetworkReply *reply);
    void onToggleUserActiveResponse(QNetworkReply *reply);
    void onBlockedUsersReceived(QNetworkReply *reply);
    void onAccessLogReceived(QNetworkReply *reply);
    void onBooksListReceived(QNetworkReply *reply);
    void onDeleteBookResponse(QNetworkReply *reply);
    void onToggleBookStatusResponse(QNetworkReply *reply);
    void onReviewsListReceived(QNetworkReply *reply);
    void onApproveReviewResponse(QNetworkReply *reply);
    void onRejectReviewResponse(QNetworkReply *reply);
    void onDeleteReviewResponse(QNetworkReply *reply);
    void onFlagReviewResponse(QNetworkReply *reply);
    void onRecentReviewsReceived(QNetworkReply *reply);

private:
    Ui::AdminWindow *ui;

    // === NETWORK MANAGER ===
    QNetworkAccessManager *m_networkManager;

    // === CONFIGURATION ===
    QString m_apiBaseUrl;
    QString m_adminToken;
    int m_adminId;
    QString m_adminName;

    // === CURRENT STATE ===
    // Selected items
    int m_selectedUserId;
    UserData m_selectedUserData;
    int m_selectedBookId;
    AdminBookData m_selectedBookData;
    int m_selectedReviewId;
    AdminReviewData m_selectedReviewData;

    // Current filters
    QString m_userFilterType;  // "all", "regular", "publisher", "blocked"
    QString m_userSearchText;
    QString m_bookSearchText;
    QString m_bookStatusFilter;
    int m_publisherFilterId;
    QString m_reviewStatusFilter;
    int m_reviewRatingFilter;
    QString m_reviewSearchText;

    // Loading states
    bool m_isLoadingDashboard;
    bool m_isLoadingUsers;
    bool m_isLoadingBooks;
    bool m_isLoadingReviews;

    // === INITIALIZATION ===
    void setupConnections();
    void setupUIInitialState();
    void setupPageSwitching();

    // === PAGE NAVIGATION ===
    void switchToPage(int pageIndex);  // 0=dashboard, 1=users, etc.

    // === LOADING STATE MANAGEMENT ===
    void showDashboardLoading(bool show);
    void showUsersLoading(bool show);
    void setLoadingText(QWidget *label, const QString &text);

    // === DASHBOARD METHODS ===
    void fetchDashboardStats();
    void fetchActivityLog();
    void fetchSystemAlerts();
    void checkServerStatus();
    void populateDashboardStats(const DashboardStats &stats);
    void populateActivityLog(const QList<ActivityLogEntry> &logs);
    void populateSystemAlerts(const QStringList &alerts);
    void populateServerStatus(bool online, int onlineUsers, bool dbConnected,
                              const QString &uptime);

    // === USER MANAGEMENT METHODS ===
    void fetchUsersList(const QString &filter = "all", const QString &search = "");
    void populateUsersTable(const QList<UserData> &users);
    void updateUserSelectionState(int row);
    void updateUserActionButtons();
    void blockSelectedUser();
    void unblockSelectedUser();
    void deleteSelectedUser();
    void toggleSelectedUserActive();
    void exportUsersToCSV();
    void loginAsSelectedUser();

    // === ACCESS CONTROL METHODS ===
    void fetchBlockedUsers();
    void fetchAccessLog();
    void populateBlockedUsersTable(const QList<BlockedUserInfo> &blockedUsers);
    void populateAccessLogTable(const QList<AccessLogEntry> &logs);

    // === CONTENT MANAGEMENT METHODS ===
    void fetchBooksList(const QString &search = "", int publisherId = -1,
                        const QString &status = "");
    void populateBooksTable(const QList<AdminBookData> &books);
    void updateBookSelectionState(int row);
    void updateBookActionButtons();
    void flagSelectedBook();
    void deleteSelectedBook();
    void toggleSelectedBookStatus();
    void fetchRecentReviewsForContentTab();

    // === REVIEWS MONITORING METHODS ===
    void fetchReviewsList(const QString &statusFilter = "",
                          int ratingFilter = 0,
                          const QString &search = "");
    void populateReviewsTable(const QList<AdminReviewData> &reviews);
    void updateReviewSelectionState(int row);
    void updateReviewActionButtons();
    void approveSelectedReview();
    void rejectSelectedReview();
    void flagSelectedReview();
    void deleteSelectedReview();
    void replyToReviewer();

    // === SERVER API CALLS ===
    // Dashboard
    QString buildDashboardStatsUrl() const;
    QString buildActivityLogUrl() const;
    QString buildAlertsUrl() const;
    QString buildServerStatusUrl() const;

    // Users
    QString buildUsersListUrl(const QString &filter, const QString &search) const;
    QString buildBlockUserUrl(int userId) const;
    QString buildUnblockUserUrl(int userId) const;
    QString buildDeleteUserUrl(int userId) const;
    QString buildToggleUserActiveUrl(int userId) const;

    // Access Control
    QString buildBlockedUsersUrl() const;
    QString buildAccessLogUrl() const;

    // Books
    QString buildBooksListUrl(const QString &search, int pubId, const QString &status) const;
    QString buildDeleteBookUrl(int bookId) const;
    QString buildToggleBookStatusUrl(int bookId) const;
    QString buildFlagBookUrl(int bookId) const;

    // Reviews
    QString buildReviewsListUrl(const QString &status, int rating, const QString &search) const;
    QString buildApproveReviewUrl(int reviewId) const;
    QString buildRejectReviewUrl(int reviewId) const;
    QString buildDeleteReviewUrl(int reviewId) const;
    QString buildFlagReviewUrl(int reviewId) const;
    QString buildRecentReviewsUrl() const;

    // Generic API request helper
    QNetworkRequest createApiRequest(const QString &url) const;

    // === HELPER METHODS ===
    QString formatDateTime(const QString &isoDate) const;
    QString formatTimeAgo(const QString &isoDate) const;
    QString getStarString(int rating) const;
    void showError(const QString &title, const QString &message);
    void showSuccess(const QString &message);
    void showConfirmation(const QString &title, const QString &message,
                          std::function<void()> onConfirm);
    void clearTable(QTableWidget *table);
};

#endif // ADMINWINDOW_H
