#include "notificationwidget.h"
#include "Mutual/ui_notificationwidget.h"
#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"
#include "../Network-Manger/NetworkManager.h"

#include <QMessageBox>
#include <QListWidgetItem>
#include <QDateTime>
#include <QColor>

NotificationWidget::NotificationWidget(NetworkManager* networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationWidget),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &NotificationWidget::handleResponse);
    connect(ui->notificationList, &QListWidget::itemDoubleClicked,
            this, &NotificationWidget::on_notificationList_itemDoubleClicked);
}

NotificationWidget::~NotificationWidget()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &NotificationWidget::handleResponse);
    delete ui;
}

void NotificationWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadNotifications();
}

void NotificationWidget::loadNotifications()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetNotifications, params);
    m_networkManager->sendRequest(request);
}

void NotificationWidget::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();
    if (type != CommandType::GetNotifications &&
        type != CommandType::MarkNotificationRead &&
        type != CommandType::MarkAllNotificationsRead &&
        type != CommandType::ClearAllNotifications) {
        return;
    }

    if (!response.isSuccess()) {
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    // ۱. دریافت و نمایش لیست اعلان‌ها
    if (type == CommandType::GetNotifications) {
        QVariantList notifications = data["notifications"].toList();
        int count = data["count"].toInt();
        int unreadCount = data["unreadCount"].toInt();

        updateNotificationsList(notifications);
        updateBadgeAndStatus(count, unreadCount);
    }
    else if (type == CommandType::MarkNotificationRead ||
             type == CommandType::MarkAllNotificationsRead ||
             type == CommandType::ClearAllNotifications) {
        loadNotifications();
    }
}
void NotificationWidget::updateNotificationsList(const QVariantList& notifications)
{
    ui->notificationList->clear();

    for (const QVariant& var : notifications) {
        QVariantMap notif = var.toMap();

        int id = notif["notificationId"].toInt();
        QString typeStr = notif["type"].toString();
        QString title = notif["title"].toString();
        QString message = notif["message"].toString();
        bool isRead = notif["isRead"].toBool();
        QString createdAtStr = notif["createdAt"].toString();
        QDateTime dt = QDateTime::fromString(createdAtStr, Qt::ISODate);
        QString formattedDate = dt.isValid() ? dt.toString("yyyy/MM/dd HH:mm") : createdAtStr;
        QString iconStr = isRead ? "✉" : "📩";
        QString itemText = QString("%1 [%2] %3\n%4\n🕒 %5")
                               .arg(iconStr, typeStr, title, message, formattedDate);

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole + 1, isRead);
        if (!isRead) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(QBrush(QColor(255, 255, 255)));
        } else {
            item->setForeground(QBrush(QColor(160, 160, 160)));
        }

        ui->notificationList->addItem(item);
    }
}

void NotificationWidget::updateBadgeAndStatus(int totalCount, int unreadCount)
{
    ui->unreadBadgeLabel->setText(QString::number(unreadCount));
    ui->statusLabel->setText(QString("Total: %1   |   Unread: %2").arg(totalCount).arg(unreadCount));
}
void NotificationWidget::on_markReadButton_clicked()
{
    QListWidgetItem *currentItem = ui->notificationList->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, "Notice", "Please select a notification first.");
        return;
    }

    int notificationId = currentItem->data(Qt::UserRole).toInt();
    bool isRead = currentItem->data(Qt::UserRole + 1).toBool();

    if (isRead) {
        return;
    }

    markNotificationAsRead(notificationId);
}
void NotificationWidget::markNotificationAsRead(int notificationId)
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0 || notificationId <= 0) return;

    QVariantMap params;
    params["notificationId"] = notificationId;
    params["userId"] = userId;

    Request request(CommandType::MarkNotificationRead, params);
    m_networkManager->sendRequest(request);
}
void NotificationWidget::on_markAllReadButton_clicked()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::MarkAllNotificationsRead, params);
    m_networkManager->sendRequest(request);
}
void NotificationWidget::on_clearAllButton_clicked()
{
    if (ui->notificationList->count() == 0) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Clear", "Are you sure you want to clear all notifications?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes) return;

    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::ClearAllNotifications, params);
    m_networkManager->sendRequest(request);
}
void NotificationWidget::on_refreshButton_clicked()
{
    loadNotifications();
}
void NotificationWidget::on_notificationList_itemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    int notificationId = item->data(Qt::UserRole).toInt();
    bool isRead = item->data(Qt::UserRole + 1).toBool();

    if (!isRead && notificationId > 0) {
        markNotificationAsRead(notificationId);
    }
}