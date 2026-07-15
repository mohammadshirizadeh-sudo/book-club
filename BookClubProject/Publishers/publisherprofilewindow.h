#ifndef PUBLISHERPROFILEWINDOW_H
#define PUBLISHERPROFILEWINDOW_H

#include <QWidget>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class PublisherProfileWindow;
}

class PublisherProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublisherProfileWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~PublisherProfileWindow();


signals:

    void openPublisherInfoDialog();


private slots:
    void on_publisherInfoPushButton_clicked();

private:
    Ui::PublisherProfileWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // PUBLISHERPROFILEWINDOW_H
