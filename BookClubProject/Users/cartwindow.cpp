#include "cartwindow.h"
#include "ui_cartwindow.h"

CartWindow::CartWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::CartWindow)
{
    ui->setupUi(this);
    setupUI();
}

CartWindow::~CartWindow() { delete ui; }

void CartWindow::setupUI()
{
    setWindowTitle("Shopping Cart - Book Club");
    setGeometry(0, 0, 1500, 800);
}

void CartWindow::on_backButton_clicked() { close(); }

void CartWindow::on_checkoutButton_clicked() { /* Checkout */ }
