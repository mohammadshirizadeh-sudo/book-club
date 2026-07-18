#include "notificationwidget.h"
#include "Mutual/ui_notificationwidget.h"

#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Notification.h"
#include "../appWindow/SessionManager.h"

#include <QMessageBox>
#include <QListWidgetItem>

NotificationWidget::NotificationWidget(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationWidget)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    // Connect network manager response signal
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &NotificationWidget::onResponseReceived);

    // Load initial notifications
    loadNotifications();
}

NotificationWidget::~NotificationWidget()
{
    delete ui;
}

void NotificationWidget::loadNotifications()
{
    int userId = SessionManager::instance()->getUserId();
    QVariantMap data;
    data["userId"] = userId;


    Request request(CommandType::GetNotifications , data);
    m_networkManager->sendRequest(request);
}

int NotificationWidget::getUnreadCount() const
{
    int unreadCount = 0;
    for (int i = 0; i < ui->notificationList->count(); ++i) {
        QListWidgetItem *item = ui->notificationList->item(i);
        if (item && item->data(Qt::UserRole).toMap()["isRead"].toBool() == false) {
            unreadCount++;
        }
    }
    return unreadCount;
}

void NotificationWidget::updateStatusLabels()
{
    int totalCount = ui->notificationList->count();
    int unreadCount = getUnreadCount();

    ui->statusLabel->setText(QString("Total: %1 | Unread: %2").arg(totalCount).arg(unreadCount));
    ui->unreadBadgeLabel->setText(QString::number(unreadCount));

    emit notificationCountChanged(unreadCount);
}

void NotificationWidget::populateNotificationList(const QVariantList &notifications)
{
    ui->notificationList->clear();

    for (const QVariant &notifVar : notifications) {
        QVariantMap notif = notifVar.toMap();

        QListWidgetItem *item = new QListWidgetItem();

        QString displayText = QString("[%1] %2")
                                  .arg(notif["timestamp"].toString())
                                  .arg(notif["message"].toString());

        item->setText(displayText);
        item->setData(Qt::UserRole, notif);

        if (!notif["isRead"].toBool()) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }

        ui->notificationList->addItem(item);
    }

    updateStatusLabels();
}

// ===== Slot Implementations =====

void NotificationWidget::on_markReadButton_clicked()
{
    QListWidgetItem *currentItem = ui->notificationList->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "Info", "Please select a notification to mark as read.");
        return;
    }

    QVariantMap notifData = currentItem->data(Qt::UserRole).toMap();
    int notificationId = notifData["id"].toInt();
    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["notificationId"] = notificationId;
    params["userId"] = userId;

    Request request(CommandType::MarkNotificationRead, params);

    m_networkManager->sendRequest(request);
}

void NotificationWidget::on_markAllReadButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm",
        "Mark all notifications as read?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        int userId = SessionManager::instance()->getUserId();
        QVariantMap params;
        params["userId"] = userId;
        Request request(CommandType::MarkAllNotificationsRead , params);

        m_networkManager->sendRequest(request);
    }
}

void NotificationWidget::on_clearAllButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Clear",
        "Are you sure you want to clear all notifications?\nThis action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        int userId = SessionManager::instance()->getUserId();
        QVariantMap params;
        params["userId"] = userId;
        Request request(CommandType::ClearAllNotifications , params);
        m_networkManager->sendRequest(request);
    }
}

void NotificationWidget::on_refreshButton_clicked()
{
    loadNotifications();
}

void NotificationWidget::on_notificationList_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    on_markReadButton_clicked();
}

void NotificationWidget::onResponseReceived(const Response& response)
{
    switch (response.getCommandType()) {
    case CommandType::GetNotifications:
        if (response.isSuccess()) {
            populateNotificationList(response.getData()["notifications"].toList());
        } else {
            QMessageBox::warning(this, "Error", "Failed to load notifications: " + response.getMessage());
        }
        break;

    case CommandType::MarkNotificationRead:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Notification marked as read.");
            loadNotifications();
        } else {
            QMessageBox::warning(this, "Error", "Failed to mark notification as read: " + response.getMessage());
        }
        break;

    case CommandType::MarkAllNotificationsRead:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "All notifications marked as read.");
            loadNotifications();
        } else {
            QMessageBox::warning(this, "Error", "Failed to mark all as read: " + response.getMessage());
        }
        break;

    case CommandType::ClearAllNotifications:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "All notifications cleared.");
            loadNotifications();
        } else {
            QMessageBox::warning(this, "Error", "Failed to clear notifications: " + response.getMessage());
        }
        break;

    default:
        break;
    }
}
