#include "publisherprofilewindow.h"
#include "PublisherInfoDialog.h"
#include "Publishers/ui_publisherprofilewindow.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

PublisherProfileWindow::PublisherProfileWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherProfileWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
}


PublisherProfileWindow::~PublisherProfileWindow()
{
    delete ui;
}

void PublisherProfileWindow::on_publisherInfoPushButton_clicked()
{
    PublisherInfoDialog dialog(m_networkManager , this);
    dialog.exec();
}

