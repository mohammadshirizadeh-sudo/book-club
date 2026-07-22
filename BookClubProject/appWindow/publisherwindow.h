#ifndef PUBLISHERWINDOW_H
#define PUBLISHERWINDOW_H

#include <QWidget>
#include "../Network-Manger/NetworkManager.h"
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

namespace Ui {
class PublisherWindow;
}

class PublisherWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublisherWindow(NetworkManager* networkManager,QWidget *parent = nullptr);
    ~PublisherWindow();

private slots:
    void on_pubProfilePushButton_clicked();

    void handleResponse(const Response& response);

    void on_pubBooksPushButton_clicked();

    void on_newBooksPushButton_clicked();

    void on_editBooksPushButton_clicked();

    void on_bookStatsPushButton_clicked();

    void on_discountPushButton_clicked();

    void on_deactivatePushButton_clicked();

    void on_notifPushButton_clicked();

signals:
    void publisherProfileWindow();

    void myBooksWindow();
    void applydiscountWindow();
    void editWindow();
    void deactivateBook();

private:
    Ui::PublisherWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // PUBLISHERWINDOW_H
