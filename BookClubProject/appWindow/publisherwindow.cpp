#include "publisherwindow.h"
#include "appWindow/ui_publisherwindow.h"

PublisherWindow::PublisherWindow(NetworkManager* networkManager,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherWindow)
    , m_networkManager(networkManager)

{
    ui->setupUi(this);
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &PublisherWindow::handleResponse);
}

PublisherWindow::~PublisherWindow()
{
    delete ui;
}

void PublisherWindow::on_pubProfilePushButton_clicked()
{
    emit publisherProfileWindow();
}


void PublisherWindow::handleResponse(const Response& response)
{

}

void PublisherWindow::on_pubBooksPushButton_clicked()
{
    emit myBooksWindow();
}

