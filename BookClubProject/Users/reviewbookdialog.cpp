#include "reviewbookdialog.h"
#include "ui_reviewbookdialog.h"

ReviewBookDialog::ReviewBookDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReviewBookDialog)
{
    ui->setupUi(this);
}

ReviewBookDialog::~ReviewBookDialog()
{
    delete ui;
}
