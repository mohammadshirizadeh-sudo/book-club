#include "editinfodialog.h"
#include "ui_editinfodialog.h"

EditInfoDialog::EditInfoDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditInfoDialog)
{
    ui->setupUi(this);
}

EditInfoDialog::~EditInfoDialog()
{
    delete ui;
}
