#include "loginwindow.h"
#include "signWindow/ui_loginwindow.h"
#include "../appWindow/SessionManager.h"

#include <QIcon>
#include <QMessageBox>

LoginWindow::LoginWindow(NetworkManager* networkManager,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
    /*
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &LoginWindow::onResponseReceived);
*/
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_forgotpassPushButton_clicked()
{
    emit openForgotPasswordWindow();
}

void LoginWindow::on_registerPushButton_clicked()
{
    emit openRegisterWindow();
}

void LoginWindow::on_signinPushButton_clicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->PasswprdLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter your username and password.", QMessageBox::Ok);
        return;
    }

    QVariantMap params;
    params["username"] = username;
    params["password"] = password;

    Request request(CommandType::Login, params);

    // ارسال درخواست لاگین
    m_networkManager->sendRequest(request);
}

// ===== دریافت پاسخ موفقیت‌آمیز از سرور =====
void LoginWindow::handleLoginResponse(const QVariantMap& data)
{
    // استخراج اطلاعات کاربر
    int userId = data.value("userId").toInt();
    QString username = data.value("username").toString();
    QString role = data.value("role").toString();

    // ذخیره در SessionManager
    SessionManager::instance()->setCurrentUser(userId, username, role);

    QMessageBox::information(this, "Success", "Welcome " + username + "!");

    // هدایت بر اساس نقش کاربر
    if (role == "User") {
        emit openUserWindow();
    }
    else if (role == "Publisher") {
        emit openPublisherWindow();
    }
    else if (role == "Admin") {
        emit openAdminWindow();
    }
    else {
        QMessageBox::critical(this, "Error", "Role " + role + " is invalid.");
    }
}

// ===== دریافت خطا از سرور =====
void LoginWindow::handleError(const QString& message)
{
    QMessageBox::critical(this, "Login error", message);
}