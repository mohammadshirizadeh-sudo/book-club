#include "favoritebookswindow.h"
#include "ui_favoritebookswindow.h"

FavoriteBooksWindow::FavoriteBooksWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FavoriteBooksWindow)
{
    ui->setupUi(this);
}

FavoriteBooksWindow::~FavoriteBooksWindow()
{
    delete ui;
}
