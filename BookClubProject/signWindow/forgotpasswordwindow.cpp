#include "forgotpasswordwindow.h"
#include "ui_forgotpasswordwindow.h"

ForgotPasswordWindow::ForgotPasswordWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ForgotPasswordWindow)
{
    ui->setupUi(this);
}

ForgotPasswordWindow::~ForgotPasswordWindow()
{
    delete ui;
}

void ForgotPasswordWindow::on_tokenPushButton_clicked()
{

}