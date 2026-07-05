#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QIcon>

LoginWindow::LoginWindow(NetworkManager* networkManager,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow), m_networkManager(networkManager)
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

/*
void LoginWindow::on_signinPushButton_clicked()
{
    emit openMainWindow();
}
*/
/*
void LoginWindow::on_signinPushButton_clicked()
{


    QString username = ui->usernameLineEdit->text();
    QString password = ui->PasswprdLineEdit->text();



    QVariantMap params;
    params["username"] = username;
    params["password"] = password;

    Request request(CommandType::Login, params);
    m_networkManager->sendRequest(request);



}
*/











