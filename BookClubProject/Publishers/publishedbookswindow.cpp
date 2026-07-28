#include "publishedbookswindow.h"
#include "Publishers/ui_publishedbookswindow.h"

PublishedBooksWindow::PublishedBooksWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublishedBooksWindow)
{
    ui->setupUi(this);
}

PublishedBooksWindow::~PublishedBooksWindow()
{
    delete ui;
}
