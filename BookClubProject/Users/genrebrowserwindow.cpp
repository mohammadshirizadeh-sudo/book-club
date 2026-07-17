#include "genrebrowserwindow.h"
#include "Users/ui_genrebrowserwindow.h"

GenreBrowserWindow::GenreBrowserWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::GenreBrowserWindow)
{
    ui->setupUi(this);
    setupUI();
}

GenreBrowserWindow::~GenreBrowserWindow() { delete ui; }

void GenreBrowserWindow::setupUI()
{
    setWindowTitle("Browse by Genre - Book Club");
    setGeometry(0, 0, 1500, 800);
}

void GenreBrowserWindow::on_backButton_clicked()
{
    close();
}