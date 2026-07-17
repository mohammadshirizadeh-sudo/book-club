
#include "mybooks.h"
#include "Publishers/ui_mybooks.h"
#include "../Publishers/addbookdialog.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Publishers/mybooks.h"

#include "../Server/Request.h"
#include "../Server/Response.h"

MyBooks::MyBooks(NetworkManager* networkManager,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyBooks)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
}

MyBooks::~MyBooks()
{
    delete ui;
}

void MyBooks::on_addBookPushButton_clicked()
{

    AddBookDialog dialog(m_networkManager , this);
    // EditInfoDialog dialog(m_networkManager , this);
    dialog.exec();

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

