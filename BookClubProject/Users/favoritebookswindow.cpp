#include "favoritebookswindow.h"
#include "Users/ui_favoritebookswindow.h"

FavoriteBooksWindow::FavoriteBooksWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::FavoriteBooksWindow)
{
    ui->setupUi(this);
    setupUI();
}

FavoriteBooksWindow::~FavoriteBooksWindow() { delete ui; }

void FavoriteBooksWindow::setupUI()
{
    setWindowTitle("My Favorites - Book Club");
    setGeometry(0, 0, 1500, 800);
}

void FavoriteBooksWindow::on_backButton_clicked() { close(); }
