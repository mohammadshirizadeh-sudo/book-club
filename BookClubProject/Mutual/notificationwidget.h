#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include <QVariantMap>

class NetworkManager;
class Response;
class QListWidgetItem;

namespace Ui {
class NotificationWidget;
}

class NotificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationWidget(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~NotificationWidget();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // مدیریت مرکزی پاسخ‌های شبکه
    void handleResponse(const Response& response);

    // اسلات‌های دکمه‌ها و عناصر UI
    void on_markReadButton_clicked();
    void on_markAllReadButton_clicked();
    void on_clearAllButton_clicked();
    void on_refreshButton_clicked();
    void on_notificationList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::NotificationWidget *ui;
    NetworkManager *m_networkManager;

    // متدهای ارسال درخواست به سرور
    void loadNotifications();
    void markNotificationAsRead(int notificationId);
    void markAllNotificationsAsRead();
    void clearAllNotifications();

    // متدهای بروزرسانی رابط کاربری
    void updateNotificationsList(const QVariantList& notifications);
    void updateBadgeAndStatus(int totalCount, int unreadCount);
};

#endif // NOTIFICATIONWIDGET_H