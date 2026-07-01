#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QIcon>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
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
    emit openMainWindow();
}