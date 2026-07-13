#include "UserProfileWindow.h"
#include "Users/ui_UserProfileWindow.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Mutual/infodialog.h"
#include "../Mutual/changepassworddialog.h"
#include "../Mutual/editinfodialog.h"

UserProfileWindow::UserProfileWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserProfileWindow)
        , m_networkManager(networkManager)
{
    ui->setupUi(this);
}

UserProfileWindow::~UserProfileWindow()
{
    delete ui;
}

void UserProfileWindow::on_favGenresPushButton_clicked()
{
    emit openGenreWindow();
}


void UserProfileWindow::on_UserInfPushButton_clicked()
{
    InfoDialog dialog(m_networkManager , this);
    dialog.exec();
}


void UserProfileWindow::on_editUserInfPushButton_clicked()
{
    EditInfoDialog dialog(m_networkManager , this);
    dialog.exec();

}


void UserProfileWindow::on_changePassPushButton_clicked()
{
    ChangePasswordDialog dialog(m_networkManager , this);
    dialog.exec();
}


void UserProfileWindow::on_shoppingHistoryPushButton_clicked()
{
    emit openShoppingHistoryDialog();
}


void UserProfileWindow::on_favBooksPushButton_clicked()
{
    emit openFavBooksDialog();
}

