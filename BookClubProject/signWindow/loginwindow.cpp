#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QIcon>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
/*
    connect(ui->registerPushButton, &QPushButton::clicked, this, &LoginWindow::openRegisterWindow);
*/  //اینو واسه وصل کردن صفحه ها نوشتم ولی جواب نمیده، مثل 2 روش دیگه ای که امتحان کردم
}

LoginWindow::~LoginWindow()
{
    delete ui;
}
