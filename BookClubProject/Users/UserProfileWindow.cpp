#include "UserProfileWindow.h"
#include "Users/ui_UserProfileWindow.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"

Form::Form(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserProfileWindow)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::on_favGenresPushButton_clicked()
{
    emit openGenreWindow();
}


void Form::on_UserInfPushButton_clicked()
{
    emit openUserInfoDialog();
}


void Form::on_editUserInfPushButton_clicked()
{
    emit openEditUserInfoDialog();
}


void Form::on_changePassPushButton_clicked()
{
    emit openChangeUserPassDialog();
}


void Form::on_shoppingHistoryPushButton_clicked()
{
    emit openShoppingHistoryDialog();
}


void Form::on_favBooksPushButton_clicked()
{
    emit openFavBooksDialog();
}

