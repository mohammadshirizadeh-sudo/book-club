#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "../Server/Request.h"
#include "../Server/Commands.h"
#include "../Network-Manger/NetworkManager.h"

#include <QWidget>

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

    void openUserWindow();
    void openPublisherWindow();
    void openAdminWindow();

public slots:
    // اسلات‌های جدید برای دریافت پاسخ سرور
    void handleLoginResponse(const QVariantMap& data);
    void handleError(const QString& message);

private slots:
    void on_forgotpassPushButton_clicked();
    void on_registerPushButton_clicked();
    void on_signinPushButton_clicked();

private:
    Ui::LoginWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // LOGINWINDOW_H
