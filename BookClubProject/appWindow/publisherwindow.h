#ifndef PUBLISHERWINDOW_H
#define PUBLISHERWINDOW_H

#include <QWidget>
#include "../Network-Manger/NetworkManager.h"

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

signals:
    void publisherProfileWindow();

    void myBooksWindow();

private:
    Ui::PublisherWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // PUBLISHERWINDOW_H
