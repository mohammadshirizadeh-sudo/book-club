#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include "../Network-Manger/NetworkManager.h"

#include <QWidget>

namespace Ui {
class RegisterWindow;
}

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void openGenreWindow();
    void openUserWindow();
    void openPublisherWindow();
    void openLoginWindow();

private slots:
    void on_userSignupPushButton_clicked();
    void on_publisherSignupPushButton_clicked();
    void on_backToSigninPushButton_clicked();


private:
    Ui::RegisterWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // REGISTERWINDOW_H
