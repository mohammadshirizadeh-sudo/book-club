#include "registerwindow.h"
#include "ui_registerwindow.h"

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::on_userSignupPushButton_clicked()
{
    emit openGenreWindow();
}

void RegisterWindow::on_publisherSignupPushButton_clicked()
{
    emit openPublisherWindow();
}

void RegisterWindow::on_backToSigninPushButton_clicked()
{
    emit openLoginWindow();
}