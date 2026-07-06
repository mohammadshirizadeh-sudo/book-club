#include "UserProfileWindow.h"
#include "Users/ui_UserProfileWindow.h"

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
