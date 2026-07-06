#include "mybooks.h"
#include "Publishers/ui_mybooks.h"

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
