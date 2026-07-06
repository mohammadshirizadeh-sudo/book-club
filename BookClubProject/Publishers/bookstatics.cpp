#include "bookstatics.h"
#include "Publishers/ui_bookstatics.h"

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
