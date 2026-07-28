#include "adminwindow.h"
#include "appWindow/ui_adminwindow.h"
#include "../Network-Manger/NetworkManager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QListWidget>
#include "../appWindow/SessionManager.h"

AdminWindow::AdminWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdminWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
    setWindowTitle("🛡️ Book Club - System Admin Panel");
    setMinimumSize(1500, 800);

    if (m_networkManager) {
        setupConnections();
    } else {
        qWarning() << "AdminWindow initialized with nullptr NetworkManager!";
    }

    setupUIInitialState();
    ui->mainStackedWidget->setCurrentIndex(0);

}

AdminWindow::~AdminWindow()
{
    delete ui;
}

void AdminWindow::setAdminInfo()
{
     SessionManager::instance()->setCurrentUser(m_adminId, m_adminName, "Admin");

}

void AdminWindow::initializeFromServer()
{
    switchToPage(0);
}

// ==================== SETUP ====================

void AdminWindow::setupConnections()
{
    // Navigation
    connect(ui->dashboardButton, &QPushButton::clicked, this, &AdminWindow::handleDashboardButtonClicked);
    connect(ui->userManageButton, &QPushButton::clicked, this, &AdminWindow::handleUserManageButtonClicked);
    connect(ui->accessControlButton, &QPushButton::clicked, this, &AdminWindow::handleAccessControlButtonClicked);
    connect(ui->contentManageButton, &QPushButton::clicked, this, &AdminWindow::handleContentManageButtonClicked);
    connect(ui->reviewsMonitorButton, &QPushButton::clicked, this, &AdminWindow::handleReviewsMonitorButtonClicked);
    connect(ui->systemLogsButton, &QPushButton::clicked, this, &AdminWindow::handleSystemLogsButtonClicked);
    connect(ui->serverStatusButton, &QPushButton::clicked, this, &AdminWindow::handleServerStatusButtonClicked);
    connect(ui->signOutButton, &QPushButton::clicked, this, &AdminWindow::handleSignOutButtonClicked);

    // Dashboard actions
    connect(ui->refreshDashboardBtn, &QPushButton::clicked, this, &AdminWindow::handleRefreshDashboardClicked);
    connect(ui->exportReportBtn, &QPushButton::clicked, this, &AdminWindow::handleExportReportClicked);
    connect(ui->backupBtn, &QPushButton::clicked, this, &AdminWindow::handleBackupClicked);
    connect(ui->clearCacheBtn, &QPushButton::clicked, this, &AdminWindow::handleClearCacheClicked);
    connect(ui->broadcastMsgBtn, &QPushButton::clicked, this, &AdminWindow::handleBroadcastMsgClicked);
    connect(ui->checkServerBtn, &QPushButton::clicked, this, &AdminWindow::handleCheckServerClicked);
    connect(ui->restartServerBtn, &QPushButton::clicked, this, &AdminWindow::handleRestartServerClicked);

    // Users
    connect(ui->allUsersRadio, &QRadioButton::toggled, this, &AdminWindow::handleAllUsersRadioToggled);
    connect(ui->regularUsersRadio, &QRadioButton::toggled, this, &AdminWindow::handleRegularUsersRadioToggled);
    connect(ui->publishersRadio, &QRadioButton::toggled, this, &AdminWindow::handlePublishersRadioToggled);
    connect(ui->blockedUsersRadio, &QRadioButton::toggled, this, &AdminWindow::handleBlockedUsersRadioToggled);
    connect(ui->userSearchLineEdit, &QLineEdit::textChanged, this, &AdminWindow::handleUserSearchTextChanged);
    connect(ui->refreshUsersBtn, &QPushButton::clicked, this, &AdminWindow::handleRefreshUsersClicked);
    connect(ui->exportUsersBtn, &QPushButton::clicked, this, &AdminWindow::handleExportUsersClicked);
    connect(ui->usersTable, &QTableWidget::cellClicked, this, &AdminWindow::handleUsersTableCellClicked);
    connect(ui->viewUserDetailsBtn, &QPushButton::clicked, this, &AdminWindow::handleViewUserDetailsClicked);
    connect(ui->blockUserBtn, &QPushButton::clicked, this, &AdminWindow::handleBlockUserClicked);
    connect(ui->unblockUserBtn, &QPushButton::clicked, this, &AdminWindow::handleUnblockUserClicked);
    connect(ui->deleteUserBtn, &QPushButton::clicked, this, &AdminWindow::handleDeleteUserClicked);
    connect(ui->toggleActiveBtn, &QPushButton::clicked, this, &AdminWindow::handleToggleActiveClicked);

    // Content management
    connect(ui->bookSearchLineEdit, &QLineEdit::textChanged, this, &AdminWindow::handleBookSearchTextChanged);
    connect(ui->publisherFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWindow::handlePublisherFilterChanged);
    connect(ui->statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWindow::handleStatusFilterChanged);
    connect(ui->refreshBooksBtn, &QPushButton::clicked, this, &AdminWindow::handleRefreshBooksClicked);
    connect(ui->adminBooksTable, &QTableWidget::cellClicked, this, &AdminWindow::handleAdminBooksTableCellClicked);
    connect(ui->viewBookDetailsBtn, &QPushButton::clicked, this, &AdminWindow::handleViewBookDetailsClicked);
    connect(ui->flagBookBtn, &QPushButton::clicked, this, &AdminWindow::handleFlagBookClicked);
    connect(ui->deleteBookBtn, &QPushButton::clicked, this, &AdminWindow::handleDeleteBookClicked);
    connect(ui->deactivateBookBtn, &QPushButton::clicked, this, &AdminWindow::handleDeactivateBookClicked);
    connect(ui->editBookBtn, &QPushButton::clicked, this, &AdminWindow::handleEditBookClicked);
    connect(ui->viewReviewsBtn, &QPushButton::clicked, this, &AdminWindow::handleViewReviewsClicked);

    // Reviews
    connect(ui->reviewStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWindow::handleReviewStatusFilterChanged);
    connect(ui->reviewRatingFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWindow::handleReviewRatingFilterChanged);
    connect(ui->reviewSearchEdit, &QLineEdit::textChanged, this, &AdminWindow::handleReviewSearchTextChanged);
    connect(ui->refreshReviewsBtn, &QPushButton::clicked, this, &AdminWindow::handleRefreshReviewsClicked);
    connect(ui->reviewsMonitorTable, &QTableWidget::cellClicked, this, &AdminWindow::handleReviewsMonitorTableCellClicked);
    connect(ui->approveReviewBtn, &QPushButton::clicked, this, &AdminWindow::handleApproveReviewClicked);
    connect(ui->rejectReviewBtn, &QPushButton::clicked, this, &AdminWindow::handleRejectReviewClicked);
    connect(ui->flagReviewBtn, &QPushButton::clicked, this, &AdminWindow::handleFlagReviewClicked);
    connect(ui->deleteReviewBtn, &QPushButton::clicked, this, &AdminWindow::handleDeleteReviewClicked);

    // Network funnel
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &AdminWindow::onResponseReceived);
}

void AdminWindow::setupUIInitialState()
{
    updateUserActionButtons();
    updateBookActionButtons();
    updateReviewActionButtons();
}

void AdminWindow::switchToPage(int pageIndex)
{
    ui->mainStackedWidget->setCurrentIndex(pageIndex);
    switch (pageIndex) {
    case 0:
        requestDashboardStats();
        requestRecentActivity();
        requestSystemAlerts();
        requestServerStatus();
        break;
    case 1: requestUsersList(); break;
    case 2: requestBlockedUsers(); requestAccessLog(); break;
    case 3: requestBooksList(); requestReviewsList(5); break;
    case 4: requestReviewsList(); break;
    case 6: requestServerStatus(); break;
    default: break;
    }
}

// ==================== NAVIGATION SLOTS ====================

void AdminWindow::handleDashboardButtonClicked()      { switchToPage(0); }
void AdminWindow::handleUserManageButtonClicked()     { switchToPage(1); }
void AdminWindow::handleAccessControlButtonClicked()  { switchToPage(2); }
void AdminWindow::handleContentManageButtonClicked()  { switchToPage(3); }
void AdminWindow::handleReviewsMonitorButtonClicked() { switchToPage(4); }
void AdminWindow::handleSystemLogsButtonClicked()     { switchToPage(5); }
void AdminWindow::handleServerStatusButtonClicked()   { switchToPage(6); }

void AdminWindow::handleSignOutButtonClicked()
{
    if (QMessageBox::question(this, "Sign Out", "Sign out of admin panel?") == QMessageBox::Yes) {
        emit signOutRequested();
    }
}

// ==================== REQUEST SENDERS ====================

void AdminWindow::requestDashboardStats() {
    if (m_networkManager) m_networkManager->sendRequest(CommandType::GetSystemStats);
}

void AdminWindow::requestRecentActivity() {
    if (!m_networkManager) return;
    QVariantMap params{{"limit", 20}};
    Request request(CommandType::GetRecentActivities, params);
    m_networkManager->sendRequest(request);
}

void AdminWindow::requestSystemAlerts() {
    if (m_networkManager) m_networkManager->sendRequest(CommandType::GetSystemAlerts);
}

void AdminWindow::requestServerStatus() {
    if (m_networkManager) m_networkManager->sendRequest(CommandType::GetServerRuntimeStatus);
}

void AdminWindow::requestUsersList() {
    if (!m_networkManager) return;
    QVariantMap params{
        {"filter", m_userFilterType},
        {"search", m_userSearchText}
    };
    Request request(CommandType::GetAllUsers, params);
    m_networkManager->sendRequest(request);
}

void AdminWindow::requestBlockedUsers() {
    if (!m_networkManager) return;
    Request request(CommandType::GetBlockedUsers);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleEditBookClicked()
{
    if (m_selectedBookId <= 0)
        return;

    emit editBookRequested(m_selectedBookId);
}


void AdminWindow::handleViewReviewsClicked() {
    if (m_selectedBookId <= 0) return;
    m_reviewsBookIdFilter = m_selectedBookId;
    switchToPage(4);
}

void AdminWindow::requestAccessLog() {
    if (!m_networkManager) return;
    QVariantMap params{{"limit", 50}};
    Request request(CommandType::GetAdminAccessLog, params);
    m_networkManager->sendRequest(request);
}

void AdminWindow::requestBooksList() {
    if (!m_networkManager) return;
    QVariantMap params{
        {"search", m_bookSearchText},
        {"publisherId", m_publisherFilterId},
        {"status", m_bookStatusFilter}
    };
    Request request(CommandType::GetAdminBooks, params);
    m_networkManager->sendRequest(request);
}

void AdminWindow::requestReviewsList(int limit) {
    if (!m_networkManager) return;
    QVariantMap params{
        {"status", m_reviewStatusFilter},
        {"rating", m_reviewRatingFilter},
        {"search", m_reviewSearchText}
    };
    if (m_reviewsBookIdFilter > 0) { params["bookId"] = m_reviewsBookIdFilter; }
    if (limit > 0) params["limit"] = limit;
    Request request(CommandType::GetAdminReviews, params);
    m_networkManager->sendRequest(request);
}

// ==================== RESPONSE DISPATCHER ====================

void AdminWindow::onResponseReceived(const Response &response)
{
    switch (response.getCommandType()) {
    case CommandType::GetSystemStats:          handleDashboardStats(response); break;
    case CommandType::GetRecentActivities:     handleRecentActivity(response); break;
    case CommandType::GetSystemAlerts:         handleSystemAlerts(response); break;
    case CommandType::GetServerRuntimeStatus:  handleServerStatus(response); break;
    case CommandType::GetAllUsers:             handleUsersList(response); break;
    case CommandType::GetBlockedUsers:         handleBlockedUsers(response); break;
    case CommandType::GetAdminAccessLog:       handleAccessLog(response); break;
    case CommandType::GetAdminBooks:           handleBooksList(response); break;
    case CommandType::GetAdminReviews:         handleReviewsList(response); break;

    case CommandType::BlockUser:
        handleGenericActionResult(response, "User blocked successfully!", [this]{ requestUsersList(); });
        break;
    case CommandType::UnblockUser:
        handleGenericActionResult(response, "User unblocked successfully!", [this]{ requestUsersList(); });
        break;
    case CommandType::DeleteUser:
        handleGenericActionResult(response, "User account deleted permanently!", [this]{ requestUsersList(); });
        break;
    case CommandType::ToggleUserActiveStatus:
        handleGenericActionResult(response, "User status updated!", [this]{ requestUsersList(); });
        break;

    case CommandType::FlagBook:
        handleGenericActionResult(response, "Book flagged for review!", [this]{ requestBooksList(); });
        break;
    case CommandType::DeactivateBook:
    case CommandType::ReactivateBook:
        handleGenericActionResult(response, "Book status updated!", [this]{ requestBooksList(); });
        break;
    case CommandType::DeleteBook:
        handleGenericActionResult(response, "Book deleted permanently!", [this]{ requestBooksList(); });
        break;

    case CommandType::ApproveReview:
        handleGenericActionResult(response, "Review approved!", [this]{ requestReviewsList(); });
        break;
    case CommandType::RejectReview:
        handleGenericActionResult(response, "Review rejected!", [this]{ requestReviewsList(); });
        break;
    case CommandType::FlagReview:
        handleGenericActionResult(response, "Review flagged!", [this]{ requestReviewsList(); });
        break;
    case CommandType::DeleteReview:
        handleGenericActionResult(response, "Review deleted!", [this]{ requestReviewsList(); });
        break;

    case CommandType::BroadcastMessage:
        handleGenericActionResult(response, "Message broadcasted to all users!", nullptr);
        break;
    case CommandType::BackupDatabase:
        handleGenericActionResult(response, "Database backup completed!", nullptr);
        break;
    case CommandType::ClearServerCache:
        handleGenericActionResult(response, "Server cache cleared!", nullptr);
        break;
    case CommandType::RestartServer:
        handleGenericActionResult(response, "Server restart scheduled.", nullptr);
        break;
    case CommandType::GetBookById:
    {
        if (response.isSuccess())
        {
            BookDetailDialog dialog(
                m_networkManager,
                response.getData(),
                this
                );

            dialog.exec();
        }
        else
        {
            showError(
                "Error",
                "Failed to load book details."
                );
        }

        break;
    }
    case CommandType::SearchUsers:
    {
        if (response.isSuccess())
        {
            QVariantList users =
                response.getData()["users"].toList();

            for (const QVariant& v : users)
            {
                QVariantMap u = v.toMap();

                if (u["id"].toInt() == m_selectedUserId)
                {
                    UserDetailDialog dialog(
                        u,
                        this
                        );

                    dialog.exec();
                    break;
                }
            }
        }
        else
        {
            showError(
                "Error",
                "Failed to load user details."
                );
        }

        break;
    }
    case CommandType::UnflagBook: handleGenericActionResult( response, "Flag removed!", [this] { requestBooksList(); } ); break;

    default:
        break;
    }
}

void AdminWindow::handleGenericActionResult(const Response &r, const QString &successMsg,
                                            std::function<void()> onSuccess)
{
    if (!r.isSuccess()) {
        showError("Error", r.getMessage().isEmpty() ? "Action failed." : r.getMessage());
        return;
    }
    showSuccess(successMsg);
    if (onSuccess) onSuccess();
}

// ==================== DASHBOARD ACTIONS ====================

void AdminWindow::handleRefreshDashboardClicked()
{
    requestDashboardStats();
    requestRecentActivity();
    requestSystemAlerts();
    requestServerStatus();
}

void AdminWindow::handleDashboardStats(const Response &r)
{
    if (!r.isSuccess()) { showError("Error", "Failed to load dashboard statistics."); return; }

    DashboardStats stats;
    stats.totalUsers      = r.getData()["total_users"].toInt();
    stats.totalPublishers = r.getData()["total_publishers"].toInt();
    stats.totalBooks      = r.getData()["total_books"].toInt();
    stats.totalRevenue    = r.getData()["total_revenue"].toDouble();
    populateDashboardStats(stats);
}

void AdminWindow::handleRecentActivity(const Response &r)
{
    if (!r.isSuccess()) return;
    QList<ActivityLogEntry> logs;
    for (const QVariant &v : r.getData()["activities"].toList()) {
        ActivityLogEntry entry;
        if (v.type() == QVariant::Map) {
            QVariantMap m = v.toMap();
            entry.timestamp = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
            entry.adminName = m["adminName"].toString();
            entry.action = m["action"].toString();
            entry.targetUser = m["targetUser"].toString();
        } else {
            entry.action = v.toString();
        }
        logs.append(entry);
    }
    populateActivityLog(logs);
}

void AdminWindow::handleSystemAlerts(const Response &r)
{
    if (!r.isSuccess()) return;
    populateSystemAlerts(r.getData()["alerts"].toStringList());
}

void AdminWindow::handleServerStatus(const Response &r)
{
    bool online = r.isSuccess() && r.getData()["online"].toBool();
    ui->serverStatusLabel->setText(QString("🖥️ Server Status: %1").arg(online ? "🟢 Online" : "🔴 Offline"));
    ui->connectedUsersLabel->setText(QString("👥 Online Users: %1").arg(r.getData()["onlineUsers"].toInt()));
    ui->dbStatusLabel->setText(QString("💾 Database: %1").arg(r.getData()["dbConnected"].toBool() ? "💚 Connected" : "❌ Disconnected"));
    ui->uptimeLabel->setText(QString("⏱️ Uptime: %1").arg(r.getData()["uptime"].toString()));
    ui->lastUpdateLabel->setText(QString("Last update: %1").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
}

void AdminWindow::handleBackupClicked()
{
    showConfirmation("Database Backup", "Initiate a database backup now?",
                     [this]{
                         if (m_networkManager) m_networkManager->sendRequest(CommandType::BackupDatabase);
                     });
}

void AdminWindow::handleClearCacheClicked()
{
    showConfirmation("Clear Cache", "Clear the server cache?",
                     [this]{
                         if (m_networkManager) m_networkManager->sendRequest(CommandType::ClearServerCache);
                     });
}

void AdminWindow::handleBroadcastMsgClicked()
{
    bool ok;
    QString message = QInputDialog::getText(this, "Broadcast Message",
                                            "Enter message to broadcast to all users:",
                                            QLineEdit::Normal, "", &ok);

    if (!ok || message.trimmed().isEmpty()) return;

    QVariantMap data;
    data["message"] = message.trimmed();
    Request request(CommandType::BroadcastMessage, data);
    if (m_networkManager) m_networkManager->sendRequest(request);
}

void AdminWindow::handleCheckServerClicked() { requestServerStatus(); }

void AdminWindow::handleRestartServerClicked()
{
    auto reply = QMessageBox::warning(this, "⚠️ Restart Server",
                                      "WARNING: This will restart the server!\nAll active connections will be temporarily lost.\n\nAre you absolutely sure?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
    if (reply == QMessageBox::Yes && m_networkManager) {
        m_networkManager->sendRequest(CommandType::RestartServer);
    }
}

void AdminWindow::handleExportReportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Report", "admin_report.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
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
        showSuccess("Report exported successfully!");
    }
}

// ==================== USER MANAGEMENT ACTIONS ====================

void AdminWindow::handleAllUsersRadioToggled(bool checked)     { if (checked) { m_userFilterType = "all"; requestUsersList(); } }
void AdminWindow::handleRegularUsersRadioToggled(bool checked) { if (checked) { m_userFilterType = "regular"; requestUsersList(); } }
void AdminWindow::handlePublishersRadioToggled(bool checked)   { if (checked) { m_userFilterType = "publisher"; requestUsersList(); } }
void AdminWindow::handleBlockedUsersRadioToggled(bool checked) { if (checked) { m_userFilterType = "blocked"; requestUsersList(); } }

void AdminWindow::handleUserSearchTextChanged(const QString &text)
{
    m_userSearchText = text;
    requestUsersList();
}

void AdminWindow::handleRefreshUsersClicked() { requestUsersList(); }

void AdminWindow::handleExportUsersClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Users", "users.csv", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { showError("Export Error", "Could not create file."); return; }
    QTextStream out(&file);
    out << "ID,Full Name,Email,Type,Status,Registered,Last Login\n";
    for (int row = 0; row < ui->usersTable->rowCount(); row++) {
        QStringList rowData;
        for (int col = 0; col < 8; col++) {
            auto *item = ui->usersTable->item(row, col);
            rowData << (item ? item->text() : "");
        }
        out << rowData.join(",") << "\n";
    }
    showSuccess(QString("Exported %1 users to CSV").arg(ui->usersTable->rowCount()));
}

void AdminWindow::handleUsersTableCellClicked(int row, int) { updateUserSelectionState(row); }

void AdminWindow::handleViewUserDetailsClicked()
{
    if (m_selectedUserId <= 0 || !m_networkManager)
        return;

    QVariantMap params;

    params["keyword"] = m_selectedUserData.email;

    Request request(
        CommandType::SearchUsers,
        params
        );

    m_networkManager->sendRequest(request);
}

void AdminWindow::handleBlockUserClicked()
{
    if (m_selectedUserId <= 0 || !m_networkManager) return;
    QVariantMap data;
    data["userId"] = m_selectedUserId;
    data["reason"] = "Blocked by admin";
    Request request(CommandType::BlockUser, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleUnblockUserClicked()
{
    if (m_selectedUserId <= 0 || !m_networkManager) return;
    QVariantMap data;
    data["userId"] = m_selectedUserId;
    Request request(CommandType::UnblockUser, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleDeleteUserClicked()
{
    if (m_selectedUserId <= 0 || !m_networkManager) return;
    auto reply = QMessageBox::warning(this, "⚠️ PERMANENT DELETE WARNING",
                                      QString("Permanently delete \"%1\" (%2)?\nThis cannot be undone.")
                                          .arg(m_selectedUserData.fullName, m_selectedUserData.email),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        QVariantMap data;
        data["userId"] = m_selectedUserId;
        Request request(CommandType::DeleteUser, data);
        m_networkManager->sendRequest(request);
    }
}

void AdminWindow::handleToggleActiveClicked()
{
    if (m_selectedUserId <= 0 || !m_networkManager) return;
    QString action = (m_selectedUserData.status == "active") ? "deactivate" : "activate";
    QVariantMap data;
    data["userId"] = m_selectedUserId;
    data["action"] = action;
    Request request(CommandType::ToggleUserActiveStatus, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleUsersList(const Response &r)
{
    if (!r.isSuccess()) { showError("Error", "Failed to load users list."); return; }
    QList<UserData> users;
    for (const QVariant &v : r.getData()["users"].toList()) {
        QVariantMap m = v.toMap();
        UserData u;
        u.id = m["id"].toInt();
        u.fullName = m["fullName"].toString();
        u.email = m["email"].toString();
        u.type = m["role"].toString().toLower();
        u.status = m["status"].toString();
        u.registeredDate = m["registered_at"].toString();
        u.lastLoginDate = m["last_login_at"].toString();
        users.append(u);
    }
    populateUsersTable(users);
}

// ==================== ACCESS CONTROL HANDLERS ====================

void AdminWindow::handleBlockedUsers(const Response &r)
{
    if (!r.isSuccess()) return;
    QList<BlockedUserInfo> blocked;
    for (const QVariant &v : r.getData()["users"].toList()) {
        QVariantMap m = v.toMap();
        BlockedUserInfo info;
        info.userId = m["userId"].toInt();
        info.name = m["fullName"].toString();
        info.email = m["email"].toString();
        info.blockedByAdmin = m["blockedBy"].toString();
        info.reason = m["reason"].toString();
        info.blockedDate = m["blockedAt"].toString();
        blocked.append(info);
    }
    populateBlockedUsersTable(blocked);
}

void AdminWindow::handleAccessLog(const Response &r)
{
    if (!r.isSuccess()) return;
    QList<AccessLogEntry> logs;
    for (const QVariant &v : r.getData()["log"].toList()) {
        QVariantMap m = v.toMap();
        AccessLogEntry e;
        e.timestamp = QDateTime::fromString(m["timestamp"].toString(), Qt::ISODate);
        e.adminName = m["adminName"].toString();
        e.action = m["action"].toString();
        e.targetUser = m["targetUser"].toString();
        e.ipAddress = m["ipAddress"].toString();
        e.status = m["status"].toString();
        logs.append(e);
    }
    populateAccessLogTable(logs);
}

// ==================== CONTENT MANAGEMENT ACTIONS ====================

void AdminWindow::handleBookSearchTextChanged(const QString &text) { m_bookSearchText = text; requestBooksList(); }

void AdminWindow::handlePublisherFilterChanged(int)
{
    m_publisherFilterId = ui->publisherFilterCombo->currentData().toInt();
    requestBooksList();
}

void AdminWindow::handleStatusFilterChanged(int)
{
    QString status = ui->statusFilterCombo->currentText();
    if (status.contains("All")) status.clear();
    else if (status.contains("Inactive")) status = "inactive";
    else if (status.contains("Active")) status = "active";
    else if (status.contains("Flagged")) status = "flagged";

    m_bookStatusFilter = status;
    requestBooksList();
}

void AdminWindow::handleRefreshBooksClicked() { requestBooksList(); }
void AdminWindow::handleAdminBooksTableCellClicked(int row, int) { updateBookSelectionState(row); }

void AdminWindow::handleViewBookDetailsClicked()
{
    if (m_selectedBookId <= 0 || !m_networkManager)
        return;

    QVariantMap params;

    qDebug()<<"admin id is ::::::::::::::::::::::::::::: "<< m_adminId;

    params["bookId"] = m_selectedBookId;
    params["userId"] = m_adminId;

    Request request(
        CommandType::GetBookById,
        params
        );

    m_networkManager->sendRequest(request);
}

void AdminWindow::handleFlagBookClicked()
{
    if (m_selectedBookId <= 0 || !m_networkManager)
        return;

    bool currentlyFlagged =
        (m_selectedBookData.status == "flagged");

    CommandType cmd =
        currentlyFlagged
            ? CommandType::UnflagBook
            : CommandType::FlagBook;

    QVariantMap data;

    data["bookId"] = m_selectedBookId;

    Request request(cmd, data);

    m_networkManager->sendRequest(request);
}

void AdminWindow::handleDeleteBookClicked()
{
    if (m_selectedBookId <= 0 || !m_networkManager) return;
    auto reply = QMessageBox::warning(this, "Delete Book",
                                      QString("Permanently delete \"%1\"?").arg(m_selectedBookData.title),
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QVariantMap data;
        data["bookId"] = m_selectedBookId;
        data["reason"] = "Removed by admin";
        Request request(CommandType::DeleteBook, data);
        m_networkManager->sendRequest(request);
    }
}

void AdminWindow::handleDeactivateBookClicked()
{
    if (m_selectedBookId <= 0 || !m_networkManager) return;
    bool active = (m_selectedBookData.status == "active");
    CommandType cmd = active ? CommandType::DeactivateBook : CommandType::ReactivateBook;

    QVariantMap data;
    data["bookId"] = m_selectedBookId;
    Request request(cmd, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleBooksList(const Response &r)
{
    if (!r.isSuccess()) { showError("Error", "Failed to load books list."); return; }
    QList<AdminBookData> books;
    for (const QVariant &v : r.getData()["books"].toList()) {
        QVariantMap m = v.toMap();
        AdminBookData b;
        b.id = m["bookId"].toInt();
        b.title = m["title"].toString();
        b.author = m["author"].toString();
        b.publisherId = m["publisherId"].toInt();
        b.price = m["price"].toDouble();
        b.status = m["status"].toString();
        b.salesCount = m["salesCount"].toInt();
        b.averageRating = m["averageRating"].toDouble();
        books.append(b);
    }
    populateBooksTable(books);
}

// ==================== REVIEWS MONITORING ACTIONS ====================

void AdminWindow::handleReviewStatusFilterChanged(int)
{
    QString s = ui->reviewStatusFilter->currentText();
    if (s.contains("All")) m_reviewStatusFilter.clear();
    else if (s.contains("Pending")) m_reviewStatusFilter = "pending";
    else if (s.contains("Approved")) m_reviewStatusFilter = "approved";
    else if (s.contains("Rejected")) m_reviewStatusFilter = "rejected";
    else if (s.contains("Flagged")) m_reviewStatusFilter = "flagged";
    requestReviewsList();
}

void AdminWindow::handleReviewRatingFilterChanged(int index) { m_reviewRatingFilter = index; requestReviewsList(); }
void AdminWindow::handleReviewSearchTextChanged(const QString &text) { m_reviewSearchText = text; requestReviewsList(); }
void AdminWindow::handleRefreshReviewsClicked() { requestReviewsList(); }
void AdminWindow::handleReviewsMonitorTableCellClicked(int row, int) { updateReviewSelectionState(row); }

void AdminWindow::handleApproveReviewClicked()
{
    if (m_selectedReviewId <= 0 || !m_networkManager) return;
    QVariantMap data;
    data["reviewId"] = m_selectedReviewId;
    Request request(CommandType::ApproveReview, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleRejectReviewClicked()
{
    if (m_selectedReviewId <= 0 || !m_networkManager) return;
    bool ok;
    QString reason = QInputDialog::getText(this, "Reject Review", "Reason (optional):", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    QVariantMap data;
    data["reviewId"] = m_selectedReviewId;
    data["reason"] = reason;
    Request request(CommandType::RejectReview, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleFlagReviewClicked()
{
    if (m_selectedReviewId <= 0 || !m_networkManager) return;
    QVariantMap data;
    data["reviewId"] = m_selectedReviewId;
    Request request(CommandType::FlagReview, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleDeleteReviewClicked()
{
    if (m_selectedReviewId <= 0 || !m_networkManager) return;
    QVariantMap data;
    data["reviewId"] = m_selectedReviewId;
    data["userId"] = m_adminId;
    Request request(CommandType::DeleteReview, data);
    m_networkManager->sendRequest(request);
}

void AdminWindow::handleReviewsList(const Response &r)
{
    if (!r.isSuccess()) { showError("Error", "Failed to load reviews list."); return; }
    QList<AdminReviewData> reviews;
    for (const QVariant &v : r.getData()["reviews"].toList()) {
        QVariantMap m = v.toMap();
        AdminReviewData rev;
        rev.id = m["reviewId"].toInt();
        rev.bookId = m["bookId"].toInt();
        rev.reviewerId = m["userId"].toInt();
        rev.rating = m["rating"].toInt();
        rev.reviewText = m["text"].toString();
        rev.status = m["status"].toString();
        rev.isFlagged = m["isFlagged"].toBool();
        rev.date = m["createdAt"].toString();
        reviews.append(rev);
    }
    populateReviewsTable(reviews);
}


// ==================== UI HELPERS (IMPLEMENTATIONS) ====================

void AdminWindow::populateDashboardStats(const DashboardStats &stats)
{
    if (ui->totalUsersValue) ui->totalUsersValue->setText(QString::number(stats.totalUsers));
    if (ui->totalPublishersValue) ui->totalPublishersValue->setText(QString::number(stats.totalPublishers));
    if (ui->totalBooksValue) ui->totalBooksValue->setText(QString::number(stats.totalBooks));
    if (ui->totalRevenueValue) ui->totalRevenueValue->setText(QString::number(stats.totalRevenue, 'f', 2));
}

void AdminWindow::populateActivityLog(const QList<ActivityLogEntry> &logs)
{
    // Implementation heavily depends on your specific object name, checking generic patterns
    QTableWidget *table = findChild<QTableWidget*>("recentActivityTable");
    if (!table) return;

    clearTable(table);
    table->setRowCount(logs.size());
    for (int i = 0; i < logs.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(formatTimeAgo(logs[i].timestamp)));
        table->setItem(i, 1, new QTableWidgetItem(logs[i].adminName));
        table->setItem(i, 2, new QTableWidgetItem(logs[i].action));
        table->setItem(i, 3, new QTableWidgetItem(logs[i].targetUser));
    }
}
QString AdminWindow::formatTimeAgo(const QDateTime &dt) const
{
    if (!dt.isValid()) return "";

    qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 60) return "Just now";
    if (secs < 3600) return QString::number(secs / 60) + " mins ago";
    if (secs < 86400) return QString::number(secs / 3600) + " hours ago";
    return QString::number(secs / 86400) + " days ago";
}

void AdminWindow::populateSystemAlerts(const QStringList &alerts)
{
    ui->alertsTextBrowser->clear();

    for (const QString &alert : alerts)
    {
        ui->alertsTextBrowser->append(alert);
    }
}

void AdminWindow::populateUsersTable(const QList<UserData> &users)
{
    clearTable(ui->usersTable);
    ui->usersTable->setRowCount(users.size());
    for (int i = 0; i < users.size(); ++i) {
        ui->usersTable->setItem(i, 0, new QTableWidgetItem(QString::number(users[i].id)));
        ui->usersTable->setItem(i, 1, new QTableWidgetItem(users[i].fullName));
        ui->usersTable->setItem(i, 2, new QTableWidgetItem(users[i].email));
        ui->usersTable->setItem(i, 3, new QTableWidgetItem(users[i].type));
        ui->usersTable->setItem(i, 4, new QTableWidgetItem(users[i].status));
        ui->usersTable->setItem(i, 5, new QTableWidgetItem(formatDateTime(users[i].registeredDate)));
        ui->usersTable->setItem(i, 6, new QTableWidgetItem(formatTimeAgo(users[i].lastLoginDate)));
    }
    m_selectedUserId = -1;
    updateUserActionButtons();
}

void AdminWindow::populateBlockedUsersTable(const QList<BlockedUserInfo> &blockedUsers)
{
    QTableWidget *table = findChild<QTableWidget*>("blockedUsersTable");
    if (!table) return;

    clearTable(table);
    table->setRowCount(blockedUsers.size());
    for (int i = 0; i < blockedUsers.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString::number(blockedUsers[i].userId)));
        table->setItem(i, 1, new QTableWidgetItem(blockedUsers[i].name));
        table->setItem(i, 2, new QTableWidgetItem(blockedUsers[i].email));
        table->setItem(i, 3, new QTableWidgetItem(blockedUsers[i].blockedByAdmin));
        table->setItem(i, 4, new QTableWidgetItem(blockedUsers[i].reason));
        table->setItem(i, 5, new QTableWidgetItem(formatDateTime(blockedUsers[i].blockedDate)));
    }
}

void AdminWindow::populateAccessLogTable(const QList<AccessLogEntry> &logs)
{
    QTableWidget *table = findChild<QTableWidget*>("accessLogTable");
    if (!table) return;

    clearTable(table);
    table->setRowCount(logs.size());
    for (int i = 0; i < logs.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(formatTimeAgo(logs[i].timestamp)));
        table->setItem(i, 1, new QTableWidgetItem(logs[i].adminName));
        table->setItem(i, 2, new QTableWidgetItem(logs[i].action));
        table->setItem(i, 3, new QTableWidgetItem(logs[i].targetUser));
        table->setItem(i, 4, new QTableWidgetItem(logs[i].ipAddress));
        table->setItem(i, 5, new QTableWidgetItem(logs[i].status));
    }
}

void AdminWindow::populateBooksTable(const QList<AdminBookData> &books)
{
    clearTable(ui->adminBooksTable);
    ui->adminBooksTable->setRowCount(books.size());
    for (int i = 0; i < books.size(); ++i) {
        ui->adminBooksTable->setItem(i, 0, new QTableWidgetItem(QString::number(books[i].id)));
        ui->adminBooksTable->setItem(i, 1, new QTableWidgetItem(books[i].title));
        ui->adminBooksTable->setItem(i, 2, new QTableWidgetItem(books[i].author));
        ui->adminBooksTable->setItem(i, 3, new QTableWidgetItem(QString::number(books[i].price, 'f', 2)));
        ui->adminBooksTable->setItem(i, 4, new QTableWidgetItem(books[i].status));
        ui->adminBooksTable->setItem(i, 5, new QTableWidgetItem(QString::number(books[i].salesCount)));
        ui->adminBooksTable->setItem(i, 6, new QTableWidgetItem(getStarString(std::round(books[i].averageRating))));
    }
    m_selectedBookId = -1;
    updateBookActionButtons();
}

void AdminWindow::populateReviewsTable(const QList<AdminReviewData> &reviews)
{
    clearTable(ui->reviewsMonitorTable);
    ui->reviewsMonitorTable->setRowCount(reviews.size());
    for (int i = 0; i < reviews.size(); ++i) {
        ui->reviewsMonitorTable->setItem(i, 0, new QTableWidgetItem(QString::number(reviews[i].id)));
        ui->reviewsMonitorTable->setItem(i, 1, new QTableWidgetItem(QString::number(reviews[i].bookId)));
        ui->reviewsMonitorTable->setItem(i, 2, new QTableWidgetItem(QString::number(reviews[i].reviewerId)));
        ui->reviewsMonitorTable->setItem(i, 3, new QTableWidgetItem(getStarString(reviews[i].rating)));
        ui->reviewsMonitorTable->setItem(i, 4, new QTableWidgetItem(reviews[i].reviewText));
        ui->reviewsMonitorTable->setItem(i, 5, new QTableWidgetItem(reviews[i].status));
        ui->reviewsMonitorTable->setItem(i, 6, new QTableWidgetItem(formatTimeAgo(reviews[i].date)));
    }
    m_selectedReviewId = -1;
    updateReviewActionButtons();
}

void AdminWindow::updateUserSelectionState(int row)
{
    if (row < 0 || row >= ui->usersTable->rowCount())
    {
        m_selectedUserId = -1;
        ui->selectedUserLabel->setText("Selected: No user selected");
    }
    else
    {
        m_selectedUserId =
            ui->usersTable->item(row, 0)
                ->text()
                .toInt();

        m_selectedUserData.fullName =
            ui->usersTable->item(row, 1)
                ->text();

        m_selectedUserData.email =
            ui->usersTable->item(row, 2)
                ->text();

        m_selectedUserData.status =
            ui->usersTable->item(row, 4)
                ->text();

        ui->selectedUserLabel->setText(
            "Selected: " + m_selectedUserData.fullName
            );
    }

    updateUserActionButtons();
}

void AdminWindow::updateUserActionButtons()
{
    bool hasSelection = (m_selectedUserId > 0);
    if (ui->viewUserDetailsBtn) ui->viewUserDetailsBtn->setEnabled(hasSelection);
    if (ui->blockUserBtn) ui->blockUserBtn->setEnabled(hasSelection);
    if (ui->unblockUserBtn) ui->unblockUserBtn->setEnabled(hasSelection);
    if (ui->deleteUserBtn) ui->deleteUserBtn->setEnabled(hasSelection);
    if (ui->toggleActiveBtn) ui->toggleActiveBtn->setEnabled(hasSelection);
}

void AdminWindow::updateBookSelectionState(int row)
{
    if (row < 0 || row >= ui->adminBooksTable->rowCount())
    {
        m_selectedBookId = -1;
        ui->selectedBookLabel->setText("Selected: No book selected");
    }
    else
    {
        m_selectedBookId =
            ui->adminBooksTable->item(row, 0)
                ->text()
                .toInt();

        m_selectedBookData.title =
            ui->adminBooksTable->item(row, 1)
                ->text();

        m_selectedBookData.status =
            ui->adminBooksTable->item(row, 4)
                ->text();

        ui->selectedBookLabel->setText(
            "Selected: " + m_selectedBookData.title
            );
    }

    updateBookActionButtons();
}
void AdminWindow::updateBookActionButtons()
{
    bool hasSelection = (m_selectedBookId > 0);

    if (ui->viewBookDetailsBtn)
        ui->viewBookDetailsBtn->setEnabled(hasSelection);

    if (ui->editBookBtn)
        ui->editBookBtn->setEnabled(hasSelection);

    if (ui->viewReviewsBtn)
        ui->viewReviewsBtn->setEnabled(hasSelection);

    if (ui->flagBookBtn)
        ui->flagBookBtn->setEnabled(hasSelection);

    if (ui->deleteBookBtn)
        ui->deleteBookBtn->setEnabled(hasSelection);

    if (ui->deactivateBookBtn)
        ui->deactivateBookBtn->setEnabled(hasSelection);
}

void AdminWindow::updateReviewSelectionState(int row)
{
    if (row < 0 || row >= ui->reviewsMonitorTable->rowCount()) {
        m_selectedReviewId = -1;
    } else {
        m_selectedReviewId = ui->reviewsMonitorTable->item(row, 0)->text().toInt();
    }
    updateReviewActionButtons();
}

void AdminWindow::updateReviewActionButtons()
{
    bool hasSelection = (m_selectedReviewId > 0);
    if (ui->approveReviewBtn) ui->approveReviewBtn->setEnabled(hasSelection);
    if (ui->rejectReviewBtn) ui->rejectReviewBtn->setEnabled(hasSelection);
    if (ui->flagReviewBtn) ui->flagReviewBtn->setEnabled(hasSelection);
    if (ui->deleteReviewBtn) ui->deleteReviewBtn->setEnabled(hasSelection);
}

void AdminWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void AdminWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "Success", message);
}

void AdminWindow::showConfirmation(const QString &title, const QString &message, std::function<void()> onConfirm)
{
    if (QMessageBox::question(this, title, message) == QMessageBox::Yes) {
        if (onConfirm) onConfirm();
    }
}

void AdminWindow::clearTable(QTableWidget *table)
{
    if (!table) return;
    table->setRowCount(0);
    table->clearContents();
}

QString AdminWindow::formatDateTime(const QString &isoDate) const
{
    QDateTime dt = QDateTime::fromString(isoDate, Qt::ISODate);
    return dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm") : isoDate;
}

QString AdminWindow::formatTimeAgo(const QString &isoDate) const
{
    QDateTime dt = QDateTime::fromString(isoDate, Qt::ISODate);
    if (!dt.isValid()) return isoDate;

    qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 60) return "Just now";
    if (secs < 3600) return QString::number(secs / 60) + " mins ago";
    if (secs < 86400) return QString::number(secs / 3600) + " hours ago";
    return QString::number(secs / 86400) + " days ago";
}

QString AdminWindow::getStarString(int rating) const
{
    rating = std::max(0, std::min(5, rating)); // Clamp between 0 and 5
    return QString("★").repeated(rating) + QString("☆").repeated(5 - rating);
}