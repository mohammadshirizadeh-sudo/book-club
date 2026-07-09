#include "mybooks.h"
#include "Publishers/ui_mybooks.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

MyBooks::MyBooks(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyBooks)
{
    ui->setupUi(this);
}

MyBooks::~MyBooks()
{
    delete ui;
}

void MyBooks::on_addBookPushButton_clicked()
{

}


void MyBooks::on_editBookPushButton_clicked()
{

}


void MyBooks::on_staticsPushButton_clicked()
{

}


void MyBooks::on_applyDiscountPushButton_clicked()
{

}


void MyBooks::on_deleteDeactivatePushButton_clicked()
{

}


void MyBooks::on_backPushButton_clicked()
{

}

