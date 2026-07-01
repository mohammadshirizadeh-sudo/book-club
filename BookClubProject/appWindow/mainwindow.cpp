#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../signWindow/loginwindow.h"
#include "../signWindow/registerwindow.h"
#include "../signWindow/forgotpasswordwindow.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
