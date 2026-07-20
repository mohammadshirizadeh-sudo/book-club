#include "bookreviewdialog.h"
#include "ui_bookreviewdialog.h"

BookReviewDialog::BookReviewDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BookReviewDialog)
{
    ui->setupUi(this);
}

BookReviewDialog::~BookReviewDialog()
{
    delete ui;
}
