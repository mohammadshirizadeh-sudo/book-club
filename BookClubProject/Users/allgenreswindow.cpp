#include "allgenreswindow.h"
#include "ui_allgenreswindow.h"

AllGenresWindow::AllGenresWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AllGenresWindow)
{
    ui->setupUi(this);
}

AllGenresWindow::~AllGenresWindow()
{
    delete ui;
}
