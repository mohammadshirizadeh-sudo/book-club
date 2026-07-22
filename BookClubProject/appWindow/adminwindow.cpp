

/*
#include "adminwindow.h"
#include "appWindow/ui_adminwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>

// ==================== STATIC JSON PARSERS ====================

UserData UserData::fromJson(const QJsonObject &json)
{
    UserData user;
    user.id = json["id"].toInt(-1);
    user.username = json["username"].toString("");
    user.email = json["email"].toString("");
    user.fullName = json["full_name"].toString(user.username);
    user.type = json["type"].toString("regular");
    user.status = json["status"].toString("active");
    user.registeredDate = json["registered_at"].toString("");
    user.lastLoginDate = json["last_login_at"].toString("");
    user.booksCount = json["books_count"].toInt(0);
    user.totalSpent = json["total_spent"].toDouble(0.0);
    return user;
}

AdminBookData AdminBookData::fromJson(const QJsonObject &json)
{
    AdminBookData book;
    book.id = json["id"].toInt(-1);
    book.title = json["title"].toString("Unknown");
    book.author = json["author"].toString("Unknown");
    book.publisherName = json["publisher_name"].toString("N/A");
    book.publisherId = json["publisher_id"].toInt(-1);
    book.price = json["price"].toDouble(0.0);
    book.status = json["status"].toString("active");
    book.salesCount = json["sales_count"].toInt(0);
    book.averageRating = json["average_rating"].toDouble(0.0);
    book.reviewsCount = json["reviews_count"].toInt(0);
    book.publishDate = json["publish_date"].toString("");
    book.coverUrl = json["cover_url"].toString("");
    return book;
}

AdminReviewData AdminReviewData::fromJson(const QJsonObject &json)
{
    AdminReviewData review;
    review.id = json["id"].toInt(-1);
    review.bookId = json["book_id"].toInt(-1);
    review.bookTitle = json["book_title"].toString("Unknown Book");
    review.reviewerId = json["reviewer_id"].toInt(-1);
    review.reviewerName = json["reviewer_name"].toString("Anonymous");
    review.rating = json["rating"].toInt(0);
    review.reviewText = json["review_text"].toString("");
    review.date = json["created_at"].toString("");
    review.status = json["status"].toString("approved");
    review.isFlagged = json["is_flagged"].toBool(false);
    return review;
}

ActivityLogEntry ActivityLogEntry::fromJson(const QJsonObject &json)
{
    ActivityLogEntry entry;
    entry.timestamp = json["timestamp"].toString("");
    entry.user = json["user_name"].toString("System");
    entry.action = json["action"].toString("");
    entry.details = json["details"].toString("");
    return entry;
}

AccessLogEntry AccessLogEntry::fromJson(const QJsonObject &json)
{
    AccessLogEntry entry;
    entry.timestamp = json["timestamp"].toString("");
    entry.adminName = json["admin_name"].toString("Admin");
    entry.action = json["action"].toString("");
    entry.targetUser = json["target_user"].toString("");
    entry.ipAddress = json["ip_address"].toString("--");
    entry.status = json["status"].toString("success");
    return entry;
}

BlockedUserInfo BlockedUserInfo::fromJson(const QJsonObject &json)
{
    BlockedUserInfo info;
    info.userId = json["user_id"].toInt(-1);
    info.name = json["name"].toString("Unknown");
    info.email = json["email"].toString("");
    info.blockedByAdmin = json["blocked_by"].toString("Admin");
    info.reason = json["reason"].toString("No reason provided");
    info.blockedDate = json["blocked_at"].toString("");
    return info;
}

// ==================== CONSTRUCTOR / DESTRUCTOR ====================

AdminWindow::AdminWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AdminWindow),
    m_networkManager(new QNetworkAccessManager(this)),
    m_adminId(0),
    m_selectedUserId(-1),
    m_selectedBookId(-1),
    m_selectedReviewId(-1),
    m_isLoadingDashboard(false),
    m_isLoadingUsers(false),
    m_isLoadingBooks(false),
    m_isLoadingReviews(false)
{
    ui->setupUi(this);

    // Window configuration
    setWindowTitle("🛡️ Book Club - System Admin Panel");
    setMinimumSize(1500, 800);

    // Setup
    setupConnections();
    setupUIInitialState();
    setupPageSwitching();

    // Default API settings
    m_apiBaseUrl = "https://api.yourbookclub.com/admin/v1";
}

AdminWindow::~AdminWindow()
{
    delete ui;
}

// ==================== PUBLIC SETTERS ====================

void AdminWindow::setApiBaseUrl(const QString &baseUrl)
{
    m_apiBaseUrl = baseUrl;
}

void AdminWindow::setAdminToken(const QString &token)
{
    m_adminToken = token;
}

void AdminWindow::setAdminInfo(int adminId, const QString &adminName)
{
    m_adminId = adminId;
    m_adminName = adminName;
}

// ==================== MAIN ENTRY POINT ====================

void AdminWindow::initializeFromServer()
{
    qDebug() << "[Admin] Initializing from server...";

    // Switch to dashboard page
    switchToPage(0);  // Dashboard is page 0

    // Show loading state
    showDashboardLoading(true);

    // Fetch all initial data in parallel
    fetchDashboardStats();
    fetchActivityLog();
    fetchSystemAlerts();
    checkServerStatus();
}

// ==================== SETUP METHODS ====================

void AdminWindow::setupConnections()
{
    // Navigation buttons
    connect(ui->dashboardButton, &QPushButton::clicked,
            this, &AdminWindow::on_dashboardButton_clicked);
    connect(ui->userManageButton, &QPushButton::clicked,
            this, &AdminWindow::on_userManageButton_clicked);
    connect(ui->accessControlButton, &QPushButton::clicked,
            this, &AdminWindow::on_accessControlButton_clicked);
    connect(ui->contentManageButton, &QPushButton::clicked,
            this, &AdminWindow::on_contentManageButton_clicked);
    connect(ui->reviewsMonitorButton, &QPushButton::clicked,
            this, &AdminWindow::on_reviewsMonitorButton_clicked);
    connect(ui->systemLogsButton, &QPushButton::clicked,
            this, &AdminWindow::on_systemLogsButton_clicked);
    connect(ui->serverStatusButton, &QPushButton::clicked,
            this, &AdminWindow::on_serverStatusButton_clicked);
    connect(ui->notifButton, &QPushButton::clicked,
            this, &AdminWindow::on_notifButton_clicked);
    connect(ui->signOutButton, &QPushButton::clicked,
            this, &AdminWindow::on_signOutButton_clicked);

    // Dashboard actions
    connect(ui->refreshDashboardBtn, &QPushButton::clicked,
            this, &AdminWindow::on_refreshDashboardBtn_clicked);
    connect(ui->exportReportBtn, &QPushButton::clicked,
            this, &AdminWindow::on_exportReportBtn_clicked);
    connect(ui->backupBtn, &QPushButton::clicked,
            this, &AdminWindow::on_backupBtn_clicked);
    connect(ui->clearCacheBtn, &QPushButton::clicked,
            this, &AdminWindow::on_clearCacheBtn_clicked);
    connect(ui->broadcastMsgBtn, &QPushButton::clicked,
            this, &AdminWindow::on_broadcastMsgBtn_clicked);
    connect(ui->checkServerBtn, &QPushButton::clicked,
            this, &AdminWindow::on_checkServerBtn_clicked);
    connect(ui->restartServerBtn, &QPushButton::clicked,
            this, &AdminWindow::on_restartServerBtn_clicked);

    // User management
    connect(ui->allUsersRadio, &QRadioButton::toggled,
            this, &AdminWindow::on_allUsersRadio_toggled);
    connect(ui->regularUsersRadio, &QRadioButton::toggled,
            this, &AdminWindow::on_regularUsersRadio_toggled);
    connect(ui->publishersRadio, &QRadioButton::toggled,
            this, &AdminWindow::on_publishersRadio_toggled);
    connect(ui->blockedUsersRadio, &QRadioButton::toggled,
            this, &AdminWindow::on_blockedUsersRadio_toggled);
    connect(ui->userSearchLineEdit, &QLineEdit::textChanged,
            this, &AdminWindow::on_userSearchLineEdit_textChanged);
    connect(ui->refreshUsersBtn, &QPushButton::clicked,
            this, &AdminWindow::on_refreshUsersBtn_clicked);
    connect(ui->exportUsersBtn, &QPushButton::clicked,
            this, &AdminWindow::on_exportUsersBtn_clicked);
    connect(ui->usersTable, &QTableWidget::cellClicked,
            this, &AdminWindow::on_usersTable_cellClicked);
    connect(ui->viewUserDetailsBtn, &QPushButton::clicked,
            this, &AdminWindow::on_viewUserDetailsBtn_clicked);
    connect(ui->blockUserBtn, &QPushButton::clicked,
            this, &AdminWindow::on_blockUserBtn_clicked);
    connect(ui->unblockUserBtn, &QPushButton::clicked,
            this, &AdminWindow::on_unblockUserBtn_clicked);
    connect(ui->deleteUserBtn, &QPushButton::clicked,
            this, &AdminWindow::on_deleteUserBtn_clicked);
    connect(ui->toggleActiveBtn, &QPushButton::clicked,
            this, &AdminWindow::on_toggleActiveBtn_clicked);
    connect(ui->loginAsUserBtn, &QPushButton::clicked,
            this, &AdminWindow::on_loginAsUserBtn_clicked);

    // Content management
    connect(ui->bookSearchLineEdit, &QLineEdit::textChanged,
            this, &AdminWindow::on_bookSearchLineEdit_textChanged);
    connect(ui->publisherFilterCombo, QComboBox::currentIndexChanged,
            this, &AdminWindow::on_publisherFilterCombo_currentIndexChanged);
    connect(ui->statusFilterCombo, QComboBox::currentIndexChanged,
            this, &AdminWindow::on_statusFilterCombo_currentIndexChanged);
    connect(ui->refreshBooksBtn, &QPushButton::clicked,
            this, &AdminWindow::on_refreshBooksBtn_clicked);
    connect(ui->adminBooksTable, &QTableWidget::cellClicked,
            this, &AdminWindow::on_adminBooksTable_cellClicked);
    connect(ui->viewBookDetailsBtn, &QPushButton::clicked,
            this, &AdminWindow::on_viewBookDetailsBtn_clicked);
    connect(ui->editBookBtn, &QPushButton::clicked,
            this, &AdminWindow::on_editBookBtn_clicked);
    connect(ui->flagBookBtn, &QPushButton::clicked,
            this, &AdminWindow::on_flagBookBtn_clicked);
    connect(ui->deleteBookBtn, &QPushButton::clicked,
            this, &AdminWindow::on_deleteBookBtn_clicked);
    connect(ui->viewReviewsBtn, &QPushButton::clicked,
            this, &AdminWindow::on_viewReviewsBtn_clicked);
    connect(ui->deactivateBookBtn, &QPushButton::clicked,
            this, &AdminWindow::on_deactivateBookBtn_clicked);

    // Reviews monitoring
    connect(ui->reviewStatusFilter, QComboBox::currentIndexChanged,
            this, &AdminWindow::on_reviewStatusFilter_currentIndexChanged);
    connect(ui->reviewRatingFilter, QComboBox::currentIndexChanged,
            this, &AdminWindow::on_reviewRatingFilter_currentIndexChanged);
    connect(ui->reviewSearchEdit, &QLineEdit::textChanged,
            this, &AdminWindow::on_reviewSearchEdit_textChanged);
    connect(ui->refreshReviewsBtn, &QPushButton::clicked,
            this, &AdminWindow::on_refreshReviewsBtn_clicked);
    connect(ui->reviewsMonitorTable, &QTableWidget::cellClicked,
            this, &AdminWindow::on_reviewsMonitorTable_cellClicked);
    connect(ui->approveReviewBtn, &QPushButton::clicked,
            this, &AdminWindow::on_approveReviewBtn_clicked);
    connect(ui->rejectReviewBtn, &QPushButton::clicked,
            this, &AdminWindow::on_rejectReviewBtn_clicked);
    connect(ui->flagReviewBtn, &QPushButton::clicked,
            this, &AdminWindow::on_flagReviewBtn_clicked);
    connect(ui->deleteReviewBtn, &QPushButton::clicked,
            this, &AdminWindow::on_deleteReviewBtn_clicked);
    connect(ui->replyToReviewerBtn, &QPushButton::clicked,
            this, &AdminWindow::on_replyToReviewerBtn_clicked);
    connect(ui->viewReviewerProfileBtn, &QPushButton::clicked,
            this, &AdminWindow::on_viewReviewerProfileBtn_clicked);

    // Blocked users table
    connect(ui->blockedUsersTable, &QTableWidget::cellClicked,
            this, &AdminWindow::on_blockedUsersTable_cellClicked);

    // Network manager - route responses to correct handlers
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, [this](QNetworkReply *reply) {
                QString url = reply->url().toString();

                if (url.contains("/dashboard/stats")) {
                    onDashboardStatsReceived(reply);
                } else if (url.contains("/activity/log")) {
                    onActivityLogReceived(reply);
                } else if (url.contains("/alerts")) {
                    onAlertsReceived(reply);
                } else if (url.contains("/server/status")) {
                    onServerStatusReceived(reply);
                } else if (url.contains("/users/list")) {
                    onUsersListReceived(reply);
                } else if (url.contains("/users/block")) {
                    onBlockUserResponse(reply);
                } else if (url.contains("/users/unblock")) {
                    onUnblockUserResponse(reply);
                } else if (url.contains("/users/delete")) {
                    onDeleteUserResponse(reply);
                } else if (url.contains("/users/toggle-active")) {
                    onToggleUserActiveResponse(reply);
                } else if (url.contains("/users/blocked-list")) {
                    onBlockedUsersReceived(reply);
                } else if (url.contains("/access/log")) {
                    onAccessLogReceived(reply);
                } else if (url.contains("/books/list") || url.contains("/books/all")) {
                    onBooksListReceived(reply);
                } else if (url.contains("/books/delete")) {
                    onDeleteBookResponse(reply);
                } else if (url.contains("/books/toggle-status")) {
                    onToggleBookStatusResponse(reply);
                } else if (url.contains("/reviews/list")) {
                    onReviewsListReceived(reply);
                } else if (url.contains("/reviews/approve")) {
                    onApproveReviewResponse(reply);
                } else if (url.contains("/reviews/reject")) {
                    onRejectReviewResponse(reply);
                } else if (url.contains("/reviews/delete")) {
                    onDeleteReviewResponse(reply);
                } else if (url.contains("/reviews/flag")) {
                    onFlagReviewResponse(reply);
                } else if (url.contains("/reviews/recent")) {
                    onRecentReviewsReceived(reply);
                }

                reply->deleteLater();
            });
}

void AdminWindow::setupUIInitialState()
{
    // Set default filter values
    m_userFilterType = "all";

    // Disable action buttons initially
    updateUserActionButtons();
    updateBookActionButtons();
    updateReviewActionButtons();
}

void AdminWindow::setupPageSwitching()
{
    // Initial page is dashboard (index 0)
    ui->mainStackedWidget->setCurrentIndex(0);
}

// ==================== PAGE NAVIGATION ====================

void AdminWindow::switchToPage(int pageIndex)
{
    ui->mainStackedWidget->setCurrentIndex(pageIndex);

    // Load data for specific pages when switching
    switch (pageIndex) {
    case 0:  // Dashboard
        break;  // Data loaded by initializeFromServer()
    case 1:  // User Management
        showUsersLoading(true);
        fetchUsersList(m_userFilterType, m_userSearchText);
        break;
    case 2:  // Access Control
        fetchBlockedUsers();
        fetchAccessLog();
        break;
    case 3:  // Content Management
        fetchBooksList(m_bookSearchText, m_publisherFilterId, m_bookStatusFilter);
        fetchRecentReviewsForContentTab();
        break;
    case 4:  // Reviews Monitoring
        fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
        break;
    default:
        // Placeholder pages - no data loading needed yet
        break;
    }
}

// ==================== NAVIGATION SLOTS ====================

void AdminWindow::on_dashboardButton_clicked() { switchToPage(0); }
void AdminWindow::on_userManageButton_clicked() { switchToPage(1); }
void AdminWindow::on_accessControlButton_clicked() { switchToPage(2); }
void AdminWindow::on_contentManageButton_clicked() { switchToPage(3); }
void AdminWindow::on_reviewsMonitorButton_clicked() { switchToPage(4); }
void AdminWindow::on_systemLogsButton_clicked() { switchToPage(5); }
void AdminWindow::on_serverStatusButton_clicked() { switchToPage(6); }
void AdminWindow::on_notifButton_clicked() { switchToPage(7); }

void AdminWindow::on_signOutButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Sign Out",
                                  "Are you sure you want to sign out of admin panel?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        emit signOutRequested();
    }
}

// ==================== DASHBOARD SLOTS ====================

void AdminWindow::on_refreshDashboardBtn_clicked()
{
    showDashboardLoading(true);
    fetchDashboardStats();
    fetchActivityLog();
    fetchSystemAlerts();
    checkServerStatus();
}

void AdminWindow::on_exportReportBtn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Report", "admin_report.csv", "CSV Files (*.csv)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Book Club Admin Report\n";
            out << "Generated," << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";
            out << "Metric,Value\n";
            out << "Total Users," << ui->totalUsersValue->text() << "\n";
            out << "Publishers," << ui->totalPublishersValue->text() << "\n";
            out << "Total Books," << ui->totalBooksValue->text() << "\n";
            out << "Total Revenue," << ui->totalRevenueValue->text() << "\n";
            file.close();
            showSuccess("Report exported successfully!");
        }
    }
}

void AdminWindow::on_backupBtn_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Database Backup",
                                  "Are you sure you want to initiate a database backup?\n"
                                  "This may take a few moments.",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // TODO: Call backup API
        showSuccess("Database backup initiated successfully!");
    }
}

void AdminWindow::on_clearCacheBtn_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Cache",
                                  "Are you sure you want to clear the server cache?\n"
                                  "This may temporarily affect performance.",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // TODO: Call clear cache API
        showSuccess("Server cache cleared successfully!");
    }
}

void AdminWindow::on_broadcastMsgBtn_clicked()
{
    bool ok;
    QString message = QInputDialog::getText(this, "Broadcast Message",
                                            "Enter message to broadcast to all users:",
                                            QLineEdit::Normal, "", &ok);
    if (ok && !message.isEmpty()) {
        // TODO: Call broadcast API
        showSuccess("Message broadcasted to all users!");
    }
}

void AdminWindow::on_checkServerBtn_clicked()
{
    checkServerStatus();
}

void AdminWindow::on_restartServerBtn_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "⚠️ Restart Server",
                                 "WARNING: This will restart the server!\n\n"
                                 "All active connections will be temporarily lost.\n\n"
                                 "Are you absolutely sure?",
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                 QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        // TODO: Call restart API
        showSuccess("Server restart command sent. Server will be back online shortly.");
    }
}

// ==================== USER MANAGEMENT SLOTS ====================

void AdminWindow::on_allUsersRadio_toggled(bool checked)
{
    if (checked) { m_userFilterType = "all"; fetchUsersList("all", m_userSearchText); }
}
void AdminWindow::on_regularUsersRadio_toggled(bool checked)
{
    if (checked) { m_userFilterType = "regular"; fetchUsersList("regular", m_userSearchText); }
}
void AdminWindow::on_publishersRadio_toggled(bool checked)
{
    if (checked) { m_userFilterType = "publisher"; fetchUsersList("publisher", m_userSearchText); }
}
void AdminWindow::on_blockedUsersRadio_toggled(bool checked)
{
    if (checked) { m_userFilterType = "blocked"; fetchUsersList("blocked", m_userSearchText); }
}

void AdminWindow::on_userSearchLineEdit_textChanged(const QString &text)
{
    m_userSearchText = text;
    // Debounce search (in production, use QTimer)
    fetchUsersList(m_userFilterType, text);
}

void AdminWindow::on_refreshUsersBtn_clicked()
{
    fetchUsersList(m_userFilterType, m_userSearchText);
}

void AdminWindow::on_exportUsersBtn_clicked()
{
    exportUsersToCSV();
}

void AdminWindow::on_usersTable_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    updateUserSelectionState(row);
}

void AdminWindow::on_viewUserDetailsBtn_clicked()
{
    if (m_selectedUserId <= 0) return;

    emit showUserDetailsRequested(m_selectedUserId);

    // Or open details dialog inline
    QMessageBox::information(this, "User Details",
                             QString("User ID: %1\nName: %2\nEmail: %3\nType: %4\nStatus: %5")
                                 .arg(m_selectedUserData.id)
                                 .arg(m_selectedUserData.fullName)
                                 .arg(m_selectedUserData.email)
                                 .arg(m_selectedUserData.type)
                                 .arg(m_selectedUserData.status));
}

void AdminWindow::on_blockUserBtn_clicked()
{
    blockSelectedUser();
}

void AdminWindow::on_unblockUserBtn_clicked()
{
    unblockSelectedUser();
}

void AdminWindow::on_deleteUserBtn_clicked()
{
    deleteSelectedUser();
}

void AdminWindow::on_toggleActiveBtn_clicked()
{
    toggleSelectedUserActive();
}

void AdminWindow::on_loginAsUserBtn_clicked()
{
    loginAsSelectedUser();
}

// ==================== CONTENT MANAGEMENT SLOTS ====================

void AdminWindow::on_bookSearchLineEdit_textChanged(const QString &text)
{
    m_bookSearchText = text;
    fetchBooksList(text, m_publisherFilterId, m_bookStatusFilter);
}

void AdminWindow::on_publisherFilterCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    // Get publisher ID from combo box data
    int pubId = ui->publisherFilterCombo->currentData().toInt(-1);
    m_publisherFilterId = pubId;
    fetchBooksList(m_bookSearchText, pubId, m_bookStatusFilter);
}

void AdminWindow::on_statusFilterCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString status = ui->statusFilterCombo->currentText();
    if (status.contains("All")) status = "";
    else if (status.contains("Active")) status = "active";
    else if (status.contains("Inactive")) status = "inactive";
    else if (status.contains("Flagged")) status = "flagged";
    m_bookStatusFilter = status;
    fetchBooksList(m_bookSearchText, m_publisherFilterId, status);
}

void AdminWindow::on_refreshBooksBtn_clicked()
{
    fetchBooksList(m_bookSearchText, m_publisherFilterId, m_bookStatusFilter);
}

void AdminWindow::on_adminBooksTable_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    updateBookSelectionState(row);
}

void AdminWindow::on_viewBookDetailsBtn_clicked()
{
    if (m_selectedBookId <= 0) return;
    emit navigateToBookDetails(m_selectedBookId);
}

void AdminWindow::on_editBookBtn_clicked()
{
    if (m_selectedBookId <= 0) return;
    // Open edit dialog or navigate to edit page
    QMessageBox::information(this, "Edit Book",
                             QString("Opening editor for:\n\"%1\"\nby %2")
                                 .arg(m_selectedBookData.title)
                                 .arg(m_selectedBookData.author));
}

void AdminWindow::on_flagBookBtn_clicked()
{
    flagSelectedBook();
}

void AdminWindow::on_deleteBookBtn_clicked()
{
    deleteSelectedBook();
}

void AdminWindow::on_viewReviewsBtn_clicked()
{
    if (m_selectedBookId <= 0) return;
    // Navigate to reviews monitoring filtered by this book
    switchToPage(4);
}

void AdminWindow::on_deactivateBookBtn_clicked()
{
    toggleSelectedBookStatus();
}

// ==================== REVIEWS MONITORING SLOTS ====================

void AdminWindow::on_reviewStatusFilter_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString status = ui->reviewStatusFilter->currentText();
    if (status.contains("All")) m_reviewStatusFilter = "";
    else if (status.contains("Pending")) m_reviewStatusFilter = "pending";
    else if (status.contains("Approved")) m_reviewStatusFilter = "approved";
    else if (status.contains("Rejected")) m_reviewStatusFilter = "rejected";
    else if (status.contains("Flagged")) m_reviewStatusFilter = "flagged";
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::on_reviewRatingFilter_currentIndexChanged(int index)
{
    m_reviewRatingFilter = index;  // 0=all, 1-5=stars
    fetchReviewsList(m_reviewStatusFilter, index, m_reviewSearchText);
}

void AdminWindow::on_reviewSearchEdit_textChanged(const QString &text)
{
    m_reviewSearchText = text;
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, text);
}

void AdminWindow::on_refreshReviewsBtn_clicked()
{
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::on_reviewsMonitorTable_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    updateReviewSelectionState(row);
}

void AdminWindow::on_approveReviewBtn_clicked()
{
    approveSelectedReview();
}

void AdminWindow::on_rejectReviewBtn_clicked()
{
    rejectSelectedReview();
}

void AdminWindow::on_flagReviewBtn_clicked()
{
    flagSelectedReview();
}

void AdminWindow::on_deleteReviewBtn_clicked()
{
    deleteSelectedReview();
}

void AdminWindow::on_replyToReviewerBtn_clicked()
{
    replyToReviewer();
}

void AdminWindow::on_viewReviewerProfileBtn_clicked()
{
    if (m_selectedReviewData.reviewerId > 0) {
        emit showUserDetailsRequested(m_selectedReviewData.reviewerId);
    }
}

void AdminWindow::on_blockedUsersTable_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    // Handle blocked user selection if needed
}

// ==================== LOADING STATE MANAGEMENT ====================

void AdminWindow::showDashboardLoading(bool show)
{
    m_isLoadingDashboard = show;
    ui->dashboardLoadingOverlay->setVisible(show);
    ui->mainStackedWidget->widget(0)->setEnabled(!show);
}

void AdminWindow::showUsersLoading(bool show)
{
    m_isLoadingUsers = show;
    ui->usersLoadingOverlay->setVisible(show);
    ui->userManagementPage->setEnabled(!show);
}

void AdminWindow::setLoadingText(QWidget *label, const QString &text)
{
    QLabel *lbl = qobject_cast<QLabel*>(label);
    if (lbl) lbl->setText(text);
}

// ==================== DASHBOARD DATA FETCHING ====================

QString AdminWindow::buildDashboardStatsUrl() const
{
    return QString("%1/dashboard/stats").arg(m_apiBaseUrl);
}

QString AdminWindow::buildActivityLogUrl() const
{
    return QString("%1/activity/log?limit=20").arg(m_apiBaseUrl);
}

QString AdminWindow::buildAlertsUrl() const
{
    return QString("%1/alerts").arg(m_apiBaseUrl);
}

QString AdminWindow::buildServerStatusUrl() const
{
    return QString("%1/server/status").arg(m_apiBaseUrl);
}

void AdminWindow::fetchDashboardStats()
{
    QNetworkRequest request = createApiRequest(buildDashboardStatsUrl());
    qDebug() << "[Admin] Fetching dashboard stats...";
    m_networkManager->get(request);
}

void AdminWindow::fetchActivityLog()
{
    QNetworkRequest request = createApiRequest(buildActivityLogUrl());
    m_networkManager->get(request);
}

void AdminWindow::fetchSystemAlerts()
{
    QNetworkRequest request = createApiRequest(buildAlertsUrl());
    m_networkManager->get(request);
}

void AdminWindow::checkServerStatus()
{
    QNetworkRequest request = createApiRequest(buildServerStatusUrl());
    m_networkManager->get(request);
}

void AdminWindow::populateDashboardStats(const DashboardStats &stats)
{
    ui->totalUsersValue->setText(QString::number(stats.totalUsers));
    ui->totalPublishersValue->setText(QString::number(stats.totalPublishers));
    ui->totalBooksValue->setText(QString::number(stats.totalBooks));
    ui->totalRevenueValue->setText(QString("$%1").arg(stats.totalRevenue, 0, 'f', 2));
    ui->lastUpdateLabel->setText(QString("Last update: %1")
                                     .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
}

void AdminWindow::populateActivityLog(const QList<ActivityLogEntry> &logs)
{
    clearTable(ui->recentActivityTable);
    ui->recentActivityTable->setRowCount(logs.size());

    for (int i = 0; i < logs.size(); i++) {
        const ActivityLogEntry &entry = logs[i];

        QTableWidgetItem *timeItem = new QTableWidgetItem(formatTimeAgo(entry.timestamp));
        timeItem->setFont(QFont("Segoe UI", 9));
        ui->recentActivityTable->setItem(i, 0, timeItem);

        QTableWidgetItem *userItem = new QTableWidgetItem(entry.user);
        userItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->recentActivityTable->setItem(i, 1, userItem);

        QTableWidgetItem *actionItem = new QTableWidgetItem(entry.action);
        actionItem->setFont(QFont("Segoe UI", 10));
        ui->recentActivityTable->setItem(i, 2, actionItem);

        QTableWidgetItem *detailsItem = new QTableWidgetItem(entry.details);
        detailsItem->setFont(QFont("Segoe UI", 9));
        ui->recentActivityTable->setItem(i, 3, detailsItem);
    }
}

void AdminWindow::populateSystemAlerts(const QStringList &alerts)
{
    if (alerts.isEmpty()) {
        ui->alertsTextBrowser->setHtml("<p style='color: #666;'>✅ No system alerts at this time.</p>");
        return;
    }

    QString html = "<html><body>";
    for (const QString &alert : alerts) {
        html += QString("<p style='color: #dc3545; margin-bottom: 8px;'>⚠️ %1</p>").arg(alert);
    }
    html += "</body></html>";

    ui->alertsTextBrowser->setHtml(html);
}

void AdminWindow::populateServerStatus(bool online, int onlineUsers, bool dbConnected,
                                       const QString &uptime)
{
    QString statusColor = online ? "#28a745" : "#dc3545";
    QString statusText = online ? "🟢 Online" : "🔴 Offline";
    QString dbColor = dbConnected ? "#28a745" : "#dc3545";
    QString dbText = dbConnected ? "💚 Connected" : "❌ Disconnected";

    ui->serverStatusLabel->setText(QString("🖥️ Server Status: %1").arg(statusText));
    ui->connectedUsersLabel->setText(QString("👥 Online Users: %1").arg(onlineUsers));
    ui->dbStatusLabel->setText(QString("💾 Database: %1").arg(dbText));
    ui->uptimeLabel->setText(QString("⏱️ Uptime: %1").arg(uptime.isEmpty() ? "--" : uptime));

    // Update label colors based on status
    ui->serverStatusLabel->setStyleSheet(
        QString("font: 600 12pt \"Segoe UI\"; color: %1;").arg(statusColor));
    ui->dbStatusLabel->setStyleSheet(
        QString("font: 600 12pt \"Segoe UI\"; color: %1;").arg(dbColor));
}

// ==================== DASHBOARD RESPONSE HANDLERS ====================

void AdminWindow::onDashboardStatsReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[Admin] Error fetching dashboard stats:" << reply->errorString();
        showError("Error", "Failed to load dashboard statistics.");
        showDashboardLoading(false);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        showDashboardLoading(false);
        return;
    }

    QJsonObject rootObj = doc.object();
    if (rootObj.contains("error")) {
        showError("Error", rootObj["message"].toString("Failed to load stats"));
        showDashboardLoading(false);
        return;
    }

    QJsonObject statsJson = rootObj["data"].toObject();
    DashboardStats stats;
    stats.totalUsers = statsJson["total_users"].toInt(0);
    stats.totalPublishers = statsJson["total_publishers"].toInt(0);
    stats.totalBooks = statsJson["total_books"].toInt(0);
    stats.totalRevenue = statsJson["total_revenue"].toDouble(0.0);
    stats.onlineUsers = statsJson["online_users"].toInt(0);
    stats.dbConnected = statsJson["db_connected"].toBool(false);
    stats.serverOnline = statsJson["server_online"].toBool(false);
    stats.serverUptime = statsJson["uptime"].toString("");

    populateDashboardStats(stats);
    populateServerStatus(stats.serverOnline, stats.onlineUsers, stats.dbConnected, stats.serverUptime);

    qDebug() << "[Admin] Dashboard stats loaded:" << stats.totalUsers << "users";
}

void AdminWindow::onActivityLogReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray logsArray = doc.object()["data"].toArray();
    QList<ActivityLogEntry> logs;
    for (const QJsonValue &val : logsArray) {
        logs.append(ActivityLogEntry::fromJson(val.toObject()));
    }

    populateActivityLog(logs);
}

void AdminWindow::onAlertsReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray alertsArray = doc.object()["data"].toArray();
    QStringList alerts;
    for (const QJsonValue &val : alertsArray) {
        alerts.append(val.toString());
    }

    populateSystemAlerts(alerts);
}

void AdminWindow::onServerStatusReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        populateServerStatus(false, 0, false, "");
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject statusJson = doc.object()["data"].toObject();
    bool online = statusJson["online"].toBool(false);
    int onlineUsers = statusJson["online_users"].toInt(0);
    bool dbConnected = statusJson["db_connected"].toBool(false);
    QString uptime = statusJson["uptime"].toString("");

    populateServerStatus(online, onlineUsers, dbConnected, uptime);

    // Hide loading after all dashboard data loaded
    showDashboardLoading(false);
}

// ==================== USER MANAGEMENT IMPLEMENTATION ====================

QString AdminWindow::buildUsersListUrl(const QString &filter, const QString &search) const
{
    QString url = QString("%1/users/list?filter=%2&search=%3")
    .arg(m_apiBaseUrl).arg(filter).arg(search);
    return url;
}

QString AdminWindow::buildBlockUserUrl(int userId) const
{
    return QString("%1/users/%2/block").arg(m_apiBaseUrl).arg(userId);
}

QString AdminWindow::buildUnblockUserUrl(int userId) const
{
    return QString("%1/users/%2/unblock").arg(m_apiBaseUrl).arg(userId);
}

QString AdminWindow::buildDeleteUserUrl(int userId) const
{
    return QString("%1/users/%2/delete").arg(m_apiBaseUrl).arg(userId);
}

QString AdminWindow::buildToggleUserActiveUrl(int userId) const
{
    return QString("%1/users/%2/toggle-active").arg(m_apiBaseUrl).arg(userId);
}

void AdminWindow::fetchUsersList(const QString &filter, const QString &search)
{
    QNetworkRequest request = createApiRequest(buildUsersListUrl(filter, search));
    m_networkManager->get(request);
}

void AdminWindow::populateUsersTable(const QList<UserData> &users)
{
    clearTable(ui->usersTable);
    ui->usersTable->setRowCount(users.size());

    for (int i = 0; i < users.size(); i++) {
        const UserData &user = users[i];

        // Store user ID in first column for retrieval
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(user.id));
        idItem->setData(Qt::UserRole, user.id);
        idItem->setData(Qt::UserRole + 1, user.type);
        idItem->setData(Qt::UserRole + 2, user.status);
        idItem->setTextAlignment(Qt::AlignCenter);
        ui->usersTable->setItem(i, 0, idItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(user.fullName);
        nameItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->usersTable->setItem(i, 1, nameItem);

        QTableWidgetItem *emailItem = new QTableWidgetItem(user.email);
        emailItem->setTextAlignment(Qt::AlignCenter);
        ui->usersTable->setItem(i, 2, emailItem);

        // Type badge
        QString typeText = (user.type == "publisher") ? "📚 Publisher" : "👤 Regular";
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeText);
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setForeground(user.type == "publisher" ?
                                    QColor(155, 89, 182) : QColor(52, 152, 219));
        ui->usersTable->setItem(i, 3, typeItem);

        // Status badge
        QString statusIcon = "✅";
        QColor statusColor = QColor(40, 167, 69);
        if (user.status == "blocked") { statusIcon = "🚫"; statusColor = QColor(220, 53, 69); }
        else if (user.status == "inactive") { statusIcon = "⏸️"; statusColor = QColor(108, 117, 125); }

        QTableWidgetItem *statusItem = new QTableWidgetItem(QString("%1 %2").arg(statusIcon).arg(user.status));
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(statusColor);
        statusItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        ui->usersTable->setItem(i, 4, statusItem);

        QTableWidgetItem *regDateItem = new QTableWidgetItem(formatDateTime(user.registeredDate));
        regDateItem->setTextAlignment(Qt::AlignCenter);
        ui->usersTable->setItem(i, 5, regDateItem);

        QTableWidgetItem *loginItem = new QTableWidgetItem(formatTimeAgo(user.lastLoginDate));
        loginItem->setTextAlignment(Qt::AlignCenter);
        ui->usersTable->setItem(i, 6, loginItem);

        // Actions placeholder
        QTableWidgetItem *actionsItem = new QTableWidgetItem("⋮");
        actionsItem->setTextAlignment(Qt::AlignCenter);
        ui->usersTable->setItem(i, 7, actionsItem);
    }

    // Update count label
    ui->userCountLabel->setText(QString("Total: %1 users").arg(users.size()));
}

void AdminWindow::updateUserSelectionState(int row)
{
    if (row < 0 || row >= ui->usersTable->rowCount()) {
        m_selectedUserId = -1;
        ui->selectedUserLabel->setText("Selected: No user selected");
        updateUserActionButtons();
        return;
    }

    QTableWidgetItem *idItem = ui->usersTable->item(row, 0);
    if (idItem) {
        m_selectedUserId = idItem->data(Qt::UserRole).toInt();
        m_selectedUserData.id = m_selectedUserId;
        m_selectedUserData.fullName = ui->usersTable->item(row, 1)->text();
        m_selectedUserData.email = ui->usersTable->item(row, 2)->text();
        m_selectedUserData.type = idItem->data(Qt::UserRole + 1).toString();
        m_selectedUserData.status = idItem->data(Qt::UserRole + 2).toString();

        ui->selectedUserLabel->setText(
            QString("Selected: %1 (%2) - Status: %3")
                .arg(m_selectedUserData.fullName)
                .arg(m_selectedUserData.email)
                .arg(m_selectedUserData.status)
            );

        updateUserActionButtons();
    }
}

void AdminWindow::updateUserActionButtons()
{
    bool hasSelection = (m_selectedUserId > 0);
    bool isActive = (m_selectedUserData.status != "blocked");

    ui->viewUserDetailsBtn->setEnabled(hasSelection);
    ui->blockUserBtn->setEnabled(hasSelection && isActive);
    ui->unblockUserBtn->setEnabled(hasSelection && !isActive);
    ui->deleteUserBtn->setEnabled(hasSelection);
    ui->toggleActiveBtn->setEnabled(hasSelection);
    ui->loginAsUserBtn->setEnabled(hasSelection && isActive);

    // Update button texts based on current status
    if (hasSelection) {
        ui->toggleActiveBtn->setText(
            m_selectedUserData.status == "active" ? "⏸️ Deactivate" : "✅ Activate"
            );
    }
}

void AdminWindow::blockSelectedUser()
{
    if (m_selectedUserId <= 0) return;

    showConfirmation("Block User",
                     QString("Are you sure you want to block \"%1\"?\n\n"
                             "This will prevent the user from logging in and using the service.")
                         .arg(m_selectedUserData.fullName),
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildBlockUserUrl(m_selectedUserId));
                         m_networkManager->post(request, QByteArray());
                     });
}

void AdminWindow::unblockSelectedUser()
{
    if (m_selectedUserId <= 0) return;

    showConfirmation("Unblock User",
                     QString("Are you sure you want to unblock \"%1\"?")
                         .arg(m_selectedUserData.fullName),
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildUnblockUserUrl(m_selectedUserId));
                         m_networkManager->post(request, QByteArray());
                     });
}

void AdminWindow::deleteSelectedUser()
{
    if (m_selectedUserId <= 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "⚠️ PERMANENT DELETE WARNING",
                                 QString("You are about to PERMANENTLY delete user:\n\n"
                                         "\"%1\" (%2)\n\n"
                                         "This action CANNOT be undone!\n"
                                         "All their data including purchases, reviews, etc. will be deleted.")
                                     .arg(m_selectedUserData.fullName)
                                     .arg(m_selectedUserData.email),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                 QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        QNetworkRequest request = createApiRequest(buildDeleteUserUrl(m_selectedUserId));
        m_networkManager->deleteResource(request);
    }
}

void AdminWindow::toggleSelectedUserActive()
{
    if (m_selectedUserId <= 0) return;

    QString action = (m_selectedUserData.status == "active") ? "deactivate" : "activate";
    showConfirmation(action == "deactivate" ? "Deactivate User" : "Activate User",
                     QString("Are you sure you want to %1 \"%2\"?")
                         .arg(action).arg(m_selectedUserData.fullName),
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildToggleUserActiveUrl(m_selectedUserId));
                         m_networkManager->post(request, QByteArray());
                     });
}

void AdminWindow::loginAsSelectedUser()
{
    if (m_selectedUserId <= 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Login As User",
                                  QString("Do you want to log in as \"%1\"?\n\n"
                                          "This allows you to see the system from their perspective.\n"
                                          "Your admin session will be maintained.")
                                      .arg(m_selectedUserData.fullName),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // TODO: Implement login-as-user functionality
        showSuccess(QString("Switched to user view: %1").arg(m_selectedUserData.fullName));
    }
}

void AdminWindow::exportUsersToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Users", "users.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showError("Export Error", "Could not create export file.");
        return;
    }

    QTextStream out(&file);
    out << "ID,Full Name,Email,Type,Status,Registered,Last Login,Books Count,Total Spent\n";

    for (int row = 0; row < ui->usersTable->rowCount(); row++) {
        QStringList rowData;
        for (int col = 0; col < 8; col++) {
            QTableWidgetItem *item = ui->usersTable->item(row, col);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    showSuccess(QString("Exported %1 users to CSV").arg(ui->usersTable->rowCount()));
}

// ==================== USER MANAGEMENT RESPONSES ====================

void AdminWindow::onUsersListReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[Admin] Error fetching users:" << reply->errorString();
        showError("Error", "Failed to load users list.");
        showUsersLoading(false);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        showUsersLoading(false);
        return;
    }

    QJsonArray usersArray = doc.object()["data"].toArray();
    QList<UserData> users;
    for (const QJsonValue &val : usersArray) {
        users.append(UserData::fromJson(val.toObject()));
    }

    populateUsersTable(users);
    showUsersLoading(false);

    qDebug() << "[Admin] Loaded" << users.size() << "users";
}

void AdminWindow::onBlockUserResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to block user.");
        return;
    }
    showSuccess("User blocked successfully!");
    fetchUsersList(m_userFilterType, m_userSearchText);
}

void AdminWindow::onUnblockUserResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to unblock user.");
        return;
    }
    showSuccess("User unblocked successfully!");
    fetchUsersList(m_userFilterType, m_userSearchText);
}

void AdminWindow::onDeleteUserResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to delete user.");
        return;
    }
    showSuccess("User account deleted permanently!");
    fetchUsersList(m_userFilterType, m_userSearchText);
}

void AdminWindow::onToggleUserActiveResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to toggle user status.");
        return;
    }
    showSuccess("User status updated!");
    fetchUsersList(m_userFilterType, m_userSearchText);
}

// ==================== ACCESS CONTROL IMPLEMENTATION ====================

QString AdminWindow::buildBlockedUsersUrl() const
{
    return QString("%1/users/blocked-list").arg(m_apiBaseUrl);
}

QString AdminWindow::buildAccessLogUrl() const
{
    return QString("%1/access/log?limit=50").arg(m_apiBaseUrl);
}

void AdminWindow::fetchBlockedUsers()
{
    QNetworkRequest request = createApiRequest(buildBlockedUsersUrl());
    m_networkManager->get(request);
}

void AdminWindow::fetchAccessLog()
{
    QNetworkRequest request = createApiRequest(buildAccessLogUrl());
    m_networkManager->get(request);
}

void AdminWindow::populateBlockedUsersTable(const QList<BlockedUserInfo> &blockedUsers)
{
    clearTable(ui->blockedUsersTable);
    ui->blockedUsersTable->setRowCount(blockedUsers.size());

    for (int i = 0; i < blockedUsers.size(); i++) {
        const BlockedUserInfo &info = blockedUsers[i];

        QTableWidgetItem *userIdItem = new QTableWidgetItem(QString::number(info.userId));
        userIdItem->setData(Qt::UserRole, info.userId);
        ui->blockedUsersTable->setItem(i, 0, userIdItem);

        ui->blockedUsersTable->setItem(i, 1, new QTableWidgetItem(info.name));
        ui->blockedUsersTable->setItem(i, 2, new QTableWidgetItem(info.email));
        ui->blockedUsersTable->setItem(i, 3, new QTableWidgetItem(info.blockedByAdmin));
        ui->blockedUsersTable->setItem(i, 4, new QTableWidgetItem(info.reason));
        ui->blockedUsersTable->setItem(i, 5, new QTableWidgetItem(formatDateTime(info.blockedDate)));

        QTableWidgetItem *actionsItem = new QTableWidgetItem("✅ Unblock");
        actionsItem->setTextAlignment(Qt::AlignCenter);
        actionsItem->setForeground(QColor(76, 175, 80));
        actionsItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        ui->blockedUsersTable->setItem(i, 6, actionsItem);
    }
}

void AdminWindow::populateAccessLogTable(const QList<AccessLogEntry> &logs)
{
    clearTable(ui->accessLogTable);
    ui->accessLogTable->setRowCount(logs.size());

    for (int i = 0; i < logs.size(); i++) {
        const AccessLogEntry &entry = logs[i];

        ui->accessLogTable->setItem(i, 0, new QTableWidgetItem(formatDateTime(entry.timestamp)));
        ui->accessLogTable->setItem(i, 1, new QTableWidgetItem(entry.adminName));
        ui->accessLogTable->setItem(i, 2, new QTableWidgetItem(entry.action));
        ui->accessLogTable->setItem(i, 3, new QTableWidgetItem(entry.targetUser));
        ui->accessLogTable->setItem(i, 4, new QTableWidgetItem(entry.ipAddress));

        QTableWidgetItem *statusItem = new QTableWidgetItem(entry.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        if (entry.status == "success") {
            statusItem->setForeground(QColor(40, 167, 69));
        } else {
            statusItem->setForeground(QColor(220, 53, 69));
        }
        ui->accessLogTable->setItem(i, 5, statusItem);
    }
}

void AdminWindow::onBlockedUsersReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray blockedArray = doc.object()["data"].toArray();
    QList<BlockedUserInfo> blockedUsers;
    for (const QJsonValue &val : blockedArray) {
        blockedUsers.append(BlockedUserInfo::fromJson(val.toObject()));
    }

    populateBlockedUsersTable(blockedUsers);
}

void AdminWindow::onAccessLogReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray logArray = doc.object()["data"].toArray();
    QList<AccessLogEntry> logs;
    for (const QJsonValue &val : logArray) {
        logs.append(AccessLogEntry::fromJson(val.toObject()));
    }

    populateAccessLogTable(logs);
}

// ==================== CONTENT MANAGEMENT IMPLEMENTATION ====================

QString AdminWindow::buildBooksListUrl(const QString &search, int pubId, const QString &status) const
{
    QString url = QString("%1/books/all?search=%2&publisher_id=%3&status=%4")
    .arg(m_apiBaseUrl).arg(search).arg(pubId).arg(status);
    return url;
}

QString AdminWindow::buildDeleteBookUrl(int bookId) const
{
    return QString("%1/books/%2/delete").arg(m_apiBaseUrl).arg(bookId);
}

QString AdminWindow::buildToggleBookStatusUrl(int bookId) const
{
    return QString("%1/books/%2/toggle-status").arg(m_apiBaseUrl).arg(bookId);
}

QString AdminWindow::buildFlagBookUrl(int bookId) const
{
    return QString("%1/books/%2/flag").arg(m_apiBaseUrl).arg(bookId);
}

QString AdminWindow::buildRecentReviewsUrl() const
{
    return QString("%1/reviews/recent?limit=10").arg(m_apiBaseUrl);
}

void AdminWindow::fetchBooksList(const QString &search, int pubId, const QString &status)
{
    QNetworkRequest request = createApiRequest(buildBooksListUrl(search, pubId, status));
    m_networkManager->get(request);
}

void AdminWindow::fetchRecentReviewsForContentTab()
{
    QNetworkRequest request = createApiRequest(buildRecentReviewsUrl());
    m_networkManager->get(request);
}

void AdminWindow::populateBooksTable(const QList<AdminBookData> &books)
{
    clearTable(ui->adminBooksTable);
    ui->adminBooksTable->setRowCount(books.size());

    for (int i = 0; i < books.size(); i++) {
        const AdminBookData &book = books[i];

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(book.id));
        idItem->setData(Qt::UserRole, book.id);
        idItem->setData(Qt::UserRole + 1, book.status);
        idItem->setTextAlignment(Qt::AlignCenter);
        ui->adminBooksTable->setItem(i, 0, idItem);

        ui->adminBooksTable->setItem(i, 1, new QTableWidgetItem(book.title));
        ui->adminBooksTable->setItem(i, 2, new QTableWidgetItem(book.author));
        ui->adminBooksTable->setItem(i, 3, new QTableWidgetItem(book.publisherName));

        QTableWidgetItem *priceItem = new QTableWidgetItem(QString("$%1").arg(book.price, 0, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignCenter);
        priceItem->setForeground(QColor(33, 150, 243));
        ui->adminBooksTable->setItem(i, 4, priceItem);

        // Status badge
        QString statusIcon = "✅";
        QColor statusColor = QColor(40, 167, 69);
        if (book.status == "inactive") { statusIcon = "⏸️"; statusColor = QColor(108, 117, 125); }
        else if (book.status == "flagged") { statusIcon = "⚠️"; statusColor = QColor(241, 196, 15); }

        QTableWidgetItem *statusItem = new QTableWidgetItem(QString("%1 %2").arg(statusIcon).arg(book.status));
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(statusColor);
        ui->adminBooksTable->setItem(i, 5, statusItem);

        QTableWidgetItem *salesItem = new QTableWidgetItem(QString::number(book.salesCount));
        salesItem->setTextAlignment(Qt::AlignCenter);
        ui->adminBooksTable->setItem(i, 6, salesItem);

        QTableWidgetItem *ratingItem = new QTableWidgetItem(QString("⭐ %1").arg(book.averageRating, 0, 'f', 1));
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setForeground(QColor(255, 193, 7));
        ui->adminBooksTable->setItem(i, 7, ratingItem);

        QTableWidgetItem *actionsItem = new QTableWidgetItem("⋮");
        actionsItem->setTextAlignment(Qt::AlignCenter);
        ui->adminBooksTable->setItem(i, 8, actionsItem);
    }

    ui->booksCountLabel->setText(QString("Total: %1 books").arg(books.size()));
}

void AdminWindow::updateBookSelectionState(int row)
{
    if (row < 0 || row >= ui->adminBooksTable->rowCount()) {
        m_selectedBookId = -1;
        ui->selectedBookLabel->setText("Selected: No book selected");
        updateBookActionButtons();
        return;
    }

    QTableWidgetItem *idItem = ui->adminBooksTable->item(row, 0);
    if (idItem) {
        m_selectedBookId = idItem->data(Qt::UserRole).toInt();
        m_selectedBookData.id = m_selectedBookId;
        m_selectedBookData.title = ui->adminBooksTable->item(row, 1)->text();
        m_selectedBookData.author = ui->adminBooksTable->item(row, 2)->text();
        m_selectedBookData.status = idItem->data(Qt::UserRole + 1).toString();

        ui->selectedBookLabel->setText(
            QString("Selected: \"%1\" by %2 - Status: %3")
                .arg(m_selectedBookData.title)
                .arg(m_selectedBookData.author)
                .arg(m_selectedBookData.status)
            );

        updateBookActionButtons();
    }
}

void AdminWindow::updateBookActionButtons()
{
    bool hasSelection = (m_selectedBookId > 0);

    ui->viewBookDetailsBtn->setEnabled(hasSelection);
    ui->editBookBtn->setEnabled(hasSelection);
    ui->flagBookBtn->setEnabled(hasSelection);
    ui->deleteBookBtn->setEnabled(hasSelection);
    ui->viewReviewsBtn->setEnabled(hasSelection);
    ui->deactivateBookBtn->setEnabled(hasSelection);

    if (hasSelection) {
        ui->deactivateBookBtn->setText(
            m_selectedBookData.status == "active" ? "⏸️ Deactivate Book" : "✅ Activate Book"
            );
    }
}

void AdminWindow::flagSelectedBook()
{
    if (m_selectedBookId <= 0) return;

    showConfirmation("Flag Book",
                     QString("Are you sure you want to flag \"%1\" for review?\n\n"
                             "This marks the book as potentially inappropriate.")
                         .arg(m_selectedBookData.title),
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildFlagBookUrl(m_selectedBookId));
                         m_networkManager->post(request, QByteArray());
                     });
}

void AdminWindow::deleteSelectedBook()
{
    if (m_selectedBookId <= 0) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Delete Book",
                                 QString("Are you sure you want to permanently delete:\n\n"
                                         "\"%1\" by %2\n\n"
                                         "This will remove the book and all associated data!")
                                     .arg(m_selectedBookData.title)
                                     .arg(m_selectedBookData.author),
                                 QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QNetworkRequest request = createApiRequest(buildDeleteBookUrl(m_selectedBookId));
        m_networkManager->deleteResource(request);
    }
}

void AdminWindow::toggleSelectedBookStatus()
{
    if (m_selectedBookId <= 0) return;

    QString action = (m_selectedBookData.status == "active") ? "deactivate" : "activate";
    showConfirmation(action == "deactivate" ? "Deactivate Book" : "Activate Book",
                     QString("Are you sure you want to %1 \"%2\"?")
                         .arg(action).arg(m_selectedBookData.title),
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildToggleBookStatusUrl(m_selectedBookId));
                         m_networkManager->post(request, QByteArray());
                     });
}

// ==================== BOOK RESPONSES ====================

void AdminWindow::onBooksListReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to load books list.");
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray booksArray = doc.object()["data"].toArray();
    QList<AdminBookData> books;
    for (const QJsonValue &val : booksArray) {
        books.append(AdminBookData::fromJson(val.toObject()));
    }

    populateBooksTable(books);
    qDebug() << "[Admin] Loaded" << books.size() << "books";
}

void AdminWindow::onDeleteBookResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to delete book.");
        return;
    }
    showSuccess("Book deleted permanently!");
    fetchBooksList(m_bookSearchText, m_publisherFilterId, m_bookStatusFilter);
}

void AdminWindow::onToggleBookStatusResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to toggle book status.");
        return;
    }
    showSuccess("Book status updated!");
    fetchBooksList(m_bookSearchText, m_publisherFilterId, m_bookStatusFilter);
}

// ==================== REVIEWS MONITORING IMPLEMENTATION ====================

QString AdminWindow::buildReviewsListUrl(const QString &status, int rating, const QString &search) const
{
    QString url = QString("%1/reviews/list?status=%2&rating=%3&search=%4")
    .arg(m_apiBaseUrl).arg(status).arg(rating).arg(search);
    return url;
}

QString AdminWindow::buildApproveReviewUrl(int reviewId) const
{
    return QString("%1/reviews/%2/approve").arg(m_apiBaseUrl).arg(reviewId);
}

QString AdminWindow::buildRejectReviewUrl(int reviewId) const
{
    return QString("%1/reviews/%2/reject").arg(m_apiBaseUrl).arg(reviewId);
}

QString AdminWindow::buildDeleteReviewUrl(int reviewId) const
{
    return QString("%1/reviews/%2/delete").arg(m_apiBaseUrl).arg(reviewId);
}

QString AdminWindow::buildFlagReviewUrl(int reviewId) const
{
    return QString("%1/reviews/%2/flag").arg(m_apiBaseUrl).arg(reviewId);
}

void AdminWindow::fetchReviewsList(const QString &status, int rating, const QString &search)
{
    QNetworkRequest request = createApiRequest(buildReviewsListUrl(status, rating, search));
    m_networkManager->get(request);
}

void AdminWindow::populateReviewsTable(const QList<AdminReviewData> &reviews)
{
    clearTable(ui->reviewsMonitorTable);
    ui->reviewsMonitorTable->setRowCount(reviews.size());

    for (int i = 0; i < reviews.size(); i++) {
        const AdminReviewData &review = reviews[i];

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(review.id));
        idItem->setData(Qt::UserRole, review.id);
        idItem->setData(Qt::UserRole + 1, review.status);
        idItem->setTextAlignment(Qt::AlignCenter);
        ui->reviewsMonitorTable->setItem(i, 0, idItem);

        ui->reviewsMonitorTable->setItem(i, 1, new QTableWidgetItem(review.bookTitle));
        ui->reviewsMonitorTable->setItem(i, 2, new QTableWidgetItem(review.reviewerName));

        QTableWidgetItem *ratingItem = new QTableWidgetItem(getStarString(review.rating));
        ratingItem->setTextAlignment(Qt::AlignCenter);
        ratingItem->setForeground(QColor(255, 193, 7));
        ui->reviewsMonitorTable->setItem(i, 3, ratingItem);

        // Truncate long reviews
        QString displayText = review.reviewText;
        if (displayText.length() > 60) displayText = displayText.left(57) + "...";
        ui->reviewsMonitorTable->setItem(i, 4, new QTableWidgetItem(displayText));

        ui->reviewsMonitorTable->setItem(i, 5, new QTableWidgetItem(formatDateTime(review.date)));

        // Status badge
        QString statusText = review.status;
        QColor statusColor = QColor(108, 117, 125);
        if (review.status == "pending") { statusText = "⏳ Pending"; statusColor = QColor(241, 196, 15); }
        else if (review.status == "approved") { statusText = "✅ Approved"; statusColor = QColor(40, 167, 69); }
        else if (review.status == "rejected") { statusText = "❌ Rejected"; statusColor = QColor(220, 53, 69); }
        else if (review.status == "flagged") { statusText = "⚠️ Flagged"; statusColor = QColor(241, 196, 15); }

        QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(statusColor);
        statusItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        ui->reviewsMonitorTable->setItem(i, 6, statusItem);

        QTableWidgetItem *actionsItem = new QTableWidgetItem("⋮");
        actionsItem->setTextAlignment(Qt::AlignCenter);
        ui->reviewsMonitorTable->setItem(i, 7, actionsItem);
    }

    ui->reviewsCountLabel->setText(QString("Total: %1 reviews").arg(reviews.size()));
}

void AdminWindow::updateReviewSelectionState(int row)
{
    if (row < 0 || row >= ui->reviewsMonitorTable->rowCount()) {
        m_selectedReviewId = -1;
        ui->selectedReviewLabel->setText("Selected: No review selected");
        updateReviewActionButtons();
        return;
    }

    QTableWidgetItem *idItem = ui->reviewsMonitorTable->item(row, 0);*/
/*
    if (idItem) {
        m_selectedReviewId = idItem->data(Qt::UserRole).toInt();
        m_selectedReviewData.id = m_selectedReviewId;
        m_selectedReviewData.bookTitle = ui->reviewsMonitorTable->item(row, 1)->text();
        m_selectedReviewData.reviewerName = ui->reviewsMonitorTable->item(row, 2)->text();
        m_selectedReviewData.rating = getStarString(idItem->data(Qt::UserRole + 1).toString()).count(QChar('★'));
        m_selectedReviewData.reviewText = ui->reviewsMonitorTable->item(row, 4)->text();
        m_selectedReviewData.status = idItem->data(Qt::UserRole + 1).toString();

        ui->selectedReviewLabel->setText(
            QString("Selected: Review #%1 - \"%2\" → \"%3\" (%4)")
                .arg(m_selectedReviewId)
                .arg(m_selectedReviewData.reviewerName)
                .arg(m_selectedReviewData.bookTitle)
                .arg(m_selectedReviewData.status)
            );

        updateReviewActionButtons();
    }
}

void AdminWindow::updateReviewActionButtons()
{
    bool hasSelection = (m_selectedReviewId > 0);

    ui->approveReviewBtn->setEnabled(hasSelection);
    ui->rejectReviewBtn->setEnabled(hasSelection);
    ui->flagReviewBtn->setEnabled(hasSelection);
    ui->deleteReviewBtn->setEnabled(hasSelection);
    ui->replyToReviewerBtn->setEnabled(hasSelection);
    ui->viewReviewerProfileBtn->setEnabled(hasSelection && m_selectedReviewData.reviewerId > 0);
}

void AdminWindow::approveSelectedReview()
{
    if (m_selectedReviewId <= 0) return;

    QNetworkRequest request = createApiRequest(buildApproveReviewUrl(m_selectedReviewId));
    m_networkManager->post(request, QByteArray());
}

void AdminWindow::rejectSelectedReview()
{
    if (m_selectedReviewId <= 0) return;

    bool ok;
    QString reason = QInputDialog::getText(this, "Reject Review",
                                           "Enter reason for rejection (optional):",
                                           QLineEdit::Normal, "", &ok);

    QNetworkRequest request = createApiRequest(buildRejectReviewUrl(m_selectedReviewId));
    if (ok && !reason.isEmpty()) {
        request.setRawHeader("X-Rejection-Reason", reason.toUtf8());
    }
    m_networkManager->post(request, QByteArray());
}

void AdminWindow::flagSelectedReview()
{
    if (m_selectedReviewId <= 0) return;

    QNetworkRequest request = createApiRequest(buildFlagReviewUrl(m_selectedReviewId));
    m_networkManager->post(request, QByteArray());
}

void AdminWindow::deleteSelectedReview()
{
    if (m_selectedReviewId <= 0) return;

    showConfirmation("Delete Review",
                     "Are you sure you want to delete this review?\nThis action cannot be undone.",
                     [this]() {
                         QNetworkRequest request = createApiRequest(buildDeleteReviewUrl(m_selectedReviewId));
                         m_networkManager->deleteResource(request);
                     });
}

void AdminWindow::replyToReviewer()
{
    if (m_selectedReviewId <= 0 || m_selectedReviewData.reviewerId <= 0) return;

    bool ok;
    QString reply = QInputDialog::getMultiLineText(this, "Reply to Reviewer",
                                                   "Enter your response:",
                                                   "", &ok);
    if (ok && !reply.isEmpty()) {
        // TODO: Send reply via API
        showSuccess("Reply sent to reviewer!");
    }
}

// ==================== REVIEW RESPONSES ====================

void AdminWindow::onReviewsListReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to load reviews list.");
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray reviewsArray = doc.object()["data"].toArray();
    QList<AdminReviewData> reviews;
    for (const QJsonValue &val : reviewsArray) {
        reviews.append(AdminReviewData::fromJson(val.toObject()));
    }

    populateReviewsTable(reviews);
    qDebug() << "[Admin] Loaded" << reviews.size() << "reviews";
}

void AdminWindow::onApproveReviewResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to approve review.");
        return;
    }
    showSuccess("Review approved!");
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::onRejectReviewResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to reject review.");
        return;
    }
    showSuccess("Review rejected!");
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::onDeleteReviewResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to delete review.");
        return;
    }
    showSuccess("Review deleted!");
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::onFlagReviewResponse(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        showError("Error", "Failed to flag review.");
        return;
    }
    showSuccess("Review flagged for review!");
    fetchReviewsList(m_reviewStatusFilter, m_reviewRatingFilter, m_reviewSearchText);
}

void AdminWindow::onRecentReviewsReceived(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonArray reviewsArray = doc.object()["data"].toArray();
    // Populate the recent reviews table in content management tab
    clearTable(ui->recentReviewsTable);
    ui->recentReviewsTable->setRowCount(qMin(reviewsArray.size(), 5));

    int count = 0;
    for (const QJsonValue &val : reviewsArray) {
        if (count >= 5) break;
        AdminReviewData review = AdminReviewData::fromJson(val.toObject());

        ui->recentReviewsTable->setItem(count, 0, new QTableWidgetItem(QString::number(review.id)));
        ui->recentReviewsTable->setItem(count, 1, new QTableWidgetItem(review.bookTitle));
        ui->recentReviewsTable->setItem(count, 2, new QTableWidgetItem(review.reviewerName));
        ui->recentReviewsTable->setItem(count, 3, new QTableWidgetItem(getStarString(review.rating)));
        ui->recentReviewsTable->setItem(count, 4, new QTableWidgetItem(review.reviewText.left(40) + "..."));
        ui->recentReviewsTable->setItem(count, 5, new QTableWidgetItem(formatDateTime(review.date)));

        QString statusColor = review.isFlagged ? "#f1c40f" : "#28a745";
        QTableWidgetItem *statusItem = new QTableWidgetItem(review.isFlagged ? "⚠️ Flagged" : "✅ OK");
        statusItem->setForeground(QColor(statusColor));
        ui->recentReviewsTable->setItem(count, 6, statusItem);

        count++;
    }
}

// ==================== HELPER METHODS ====================

QNetworkRequest AdminWindow::createApiRequest(const QString &url) const
{
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    if (!m_adminToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_adminToken).toUtf8());
    }
    request.setRawHeader("X-Admin-ID", QString::number(m_adminId).toUtf8());
    request.setRawHeader("X-Admin-Name", m_adminName.toUtf8());

    return request;
}

QString AdminWindow::formatDateTime(const QString &isoDate) const
{
    if (isoDate.isEmpty()) return "--";
    QDateTime dt = QDateTime::fromString(isoDate, Qt::ISODate);
    return dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : isoDate;
}

QString AdminWindow::formatTimeAgo(const QString &isoDate) const
{
    if (isoDate.isEmpty()) return "--";
    QDateTime dt = QDateTime::fromString(isoDate, Qt::ISODate);
    if (!dt.isValid()) return isoDate;

    qint64 seconds = dt.secsTo(QDateTime::currentDateTime());
    if (seconds < 60) return "Just now";
    if (seconds < 3600) return QString("%1m ago").arg(seconds / 60);
    if (seconds < 86400) return QString("%1h ago").arg(seconds / 3600);
    if (seconds < 604800) return QString("%1d ago").arg(seconds / 86400);
    return dt.toString("MMM d");
}

QString AdminWindow::getStarString(int rating) const
{
    QString stars;
    for (int i = 0; i < 5; i++) {
        stars += (i < rating) ? "★" : "☆";
    }
    return stars;
}

void AdminWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void AdminWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "Success", message);
}

void AdminWindow::showConfirmation(const QString &title, const QString &message,
                                   std::function<void()> onConfirm)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, title, message,
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        onConfirm();
    }
}

void AdminWindow::clearTable(QTableWidget *table)
{
    table->setRowCount(0);
}
*/