#include "publisherprofilewindow.h"
#include "Publishers/ui_publisherprofilewindow.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

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
