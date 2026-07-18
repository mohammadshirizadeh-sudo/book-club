#include "shoppinghistorywindow.h"
#include "ui_shoppinghistorywindow.h"

ShoppingHistoryWindow::ShoppingHistoryWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ShoppingHistoryWindow)
{
    ui->setupUi(this);
    setupUI();
}

ShoppingHistoryWindow::~ShoppingHistoryWindow() { delete ui; }

void ShoppingHistoryWindow::setupUI()
{
    setWindowTitle("Shopping History - Book Club");
    setGeometry(0, 0, 1500, 800);
}

void ShoppingHistoryWindow::on_backButton_clicked()
{

}