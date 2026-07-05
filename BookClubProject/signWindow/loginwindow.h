#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H


#include <QWidget>
#include "../Server/Request.h"
#include "../Server/Commands.h"
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(NetworkManager* networkManager  , QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void openForgotPasswordWindow();
    void openRegisterWindow();
    //void openMainWindow();

private slots:
    void on_forgotpassPushButton_clicked();
    void on_registerPushButton_clicked();
    // void on_signinPushButton_clicked();

    // void on_signinPushButton_clicked();

private:
    Ui::LoginWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // LOGINWINDOW_H
