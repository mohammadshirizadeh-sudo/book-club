#include "edituserinfodialog.h"
#include "Users/ui_edituserinfodialog.h"

EditUserInfoDialog::EditUserInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditUserInfoDialog)
{
    ui->setupUi(this);
}

EditUserInfoDialog::~EditUserInfoDialog()
{
    delete ui;
}
