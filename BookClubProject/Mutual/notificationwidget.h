#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>
#include "../NetworkManger/NetworkManager.h"

namespace Ui {
class NotificationWidget;
}

class NotificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationWidget(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~NotificationWidget();

    void loadNotifications();
    int getUnreadCount() const;

signals:
    void notificationCountChanged(int count);

private slots:
    void on_markReadButton_clicked();
    void on_markAllReadButton_clicked();
    void on_clearAllButton_clicked();
    void on_refreshButton_clicked();
    void on_notificationList_itemDoubleClicked(QListWidgetItem *item);
    void onResponseReceived(const Response& response);

private:
    Ui::NotificationWidget *ui;
    NetworkManager* m_networkManager;

    void updateStatusLabels();
    void populateNotificationList(const QVariantList &notifications);
};

#endif // NOTIFICATIONWIDGET_H
