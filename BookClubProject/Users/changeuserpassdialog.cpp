#include "changeuserpassdialog.h"
#include "ui_changeuserpassdialog.h"

ChangeUserPassDialog::ChangeUserPassDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangeUserPassDialog)
{
    ui->setupUi(this);
}

ChangeUserPassDialog::~ChangeUserPassDialog()
{
    delete ui;
}
