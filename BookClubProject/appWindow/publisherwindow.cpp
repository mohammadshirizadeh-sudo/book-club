#include "publisherwindow.h"
#include "appWindow/ui_publisherwindow.h"

#include "SessionManager.h"
#include "Publishers/ui_addbookdialog.h"

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

}


void PublisherWindow::on_newBooksPushButton_clicked()
{
    int publisherId = SessionManager::instance()->getUserId();
    AddBookDialog dialog(m_networkManager ,publisherId );
    dialog.exec();
    // AddNewBookDialog dialog(m_networkManager , this);

    // AddBookDialog dialog(m_networkManager , this);

    // EditInfoDialog dialog(m_networkManager , this);
    // dialog.exec();
}


void PublisherWindow::on_editBooksPushButton_clicked()
{

}


void PublisherWindow::on_bookStatsPushButton_clicked()
{

}


void PublisherWindow::on_discountPushButton_clicked()
{

}


void PublisherWindow::on_deactivatePushButton_clicked()
{

}


void PublisherWindow::on_notifPushButton_clicked()
{

}

