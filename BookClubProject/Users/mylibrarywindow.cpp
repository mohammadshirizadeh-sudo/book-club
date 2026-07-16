#include "mylibrarywindow.h"
#include "Users/ui_mylibrarywindow.h"

MyLibraryWindow::MyLibraryWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyLibraryWindow)
{
    ui->setupUi(this);
}

MyLibraryWindow::~MyLibraryWindow()
{
    delete ui;
}
