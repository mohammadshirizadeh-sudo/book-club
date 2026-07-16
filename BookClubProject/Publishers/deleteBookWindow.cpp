#include "deleteBookWindow.h"
#include "ui_deleteBookWindow.h"

deleteBookWindow::deleteBookWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::deleteBookWindow)
{
    ui->setupUi(this);
}

deleteBookWindow::~deleteBookWindow()
{
    delete ui;
}
