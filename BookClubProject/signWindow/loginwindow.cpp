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
    qDebug() << "🔗 Connecting responseReceived to handleLoginResponse";
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &LoginWindow::handleLoginResponse);
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
    qDebug()<<"We are in the onsignin button";

    m_networkManager->sendRequest(request);
}
void LoginWindow::handleLoginResponse(const Response& response)
{


    if (response.getCommandType() != CommandType::Login) {
        return;
    }


    if (!response.isSuccess()) {
        QMessageBox::critical(this, "Login Error", response.getMessage());
        return;
    }


    QVariantMap data = response.getData();

    int userId = data.value("userId").toInt();
    QString username = data.value("username").toString();
    QString role = data.value("role").toString();

    SessionManager::instance()->setCurrentUser(userId, username, role);

    QMessageBox::information(this, "Success", "Welcome " + username + "!");

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

void LoginWindow::handleError(const QString& message)
{
    QMessageBox::critical(this, "Login error", message);
}