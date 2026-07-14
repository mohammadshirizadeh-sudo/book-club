#include "cartwindow.h"
#include "ui_cartwindow.h"

CartWindow::CartWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CartWindow)
{
    ui->setupUi(this);
}

CartWindow::~CartWindow()
{
    delete ui;
}
