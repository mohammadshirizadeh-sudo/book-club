#include "publisherwindow.h"
#include "appWindow/ui_publisherwindow.h"

PublisherWindow::PublisherWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherWindow)
{
    ui->setupUi(this);
}

PublisherWindow::~PublisherWindow()
{
    delete ui;
}
