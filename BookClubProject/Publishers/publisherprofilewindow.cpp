#include "publisherprofilewindow.h"
#include "Publishers/ui_publisherprofilewindow.h"

PublisherProfileWindow::PublisherProfileWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherProfileWindow)
{
    ui->setupUi(this);
}

PublisherProfileWindow::~PublisherProfileWindow()
{
    delete ui;
}
