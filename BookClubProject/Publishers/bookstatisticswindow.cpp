#include "bookstatisticswindow.h"
#include "Publishers/ui_bookstatisticswindow.h"

BookStatisticsWindow::BookStatisticsWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::BookStatisticsWindow)
{
    ui->setupUi(this);
    setupUI();
}

BookStatisticsWindow::~BookStatisticsWindow() { delete ui; }

void BookStatisticsWindow::setupUI()
{
    setWindowTitle("Book Statistics - Publisher Panel");
    setGeometry(0, 0, 1500, 800);
}

void BookStatisticsWindow::on_backButton_clicked() { close(); }
