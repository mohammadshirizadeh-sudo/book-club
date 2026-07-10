#include "bookstatics.h"
#include "Publishers/ui_bookstatics.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

BookStatics::BookStatics(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BookStatics)
{
    ui->setupUi(this);
}

BookStatics::~BookStatics()
{
    delete ui;
}
