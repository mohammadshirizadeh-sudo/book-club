#include "mylibrarywindow.h"
#include "Users/ui_mylibrarywindow.h"

MyLibraryWindow::MyLibraryWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MyLibraryWindow)
{
    ui->setupUi(this);
    setupUI();
}

MyLibraryWindow::~MyLibraryWindow() { delete ui; }

void MyLibraryWindow::setupUI()
{
    setWindowTitle("My Library - Book Club");
    setGeometry(0, 0, 1500, 800);
}

void MyLibraryWindow::on_backButton_clicked()
{
    close();
}