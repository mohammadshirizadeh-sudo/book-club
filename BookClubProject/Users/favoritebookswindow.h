#ifndef FAVORITEBOOKSWINDOW_H
#define FAVORITEBOOKSWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class FavoriteBooksWindow; }
QT_END_NAMESPACE

class FavoriteBooksWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FavoriteBooksWindow(QWidget *parent = nullptr);
    ~FavoriteBooksWindow();

private slots:
    void on_backButton_clicked();

private:
    Ui::FavoriteBooksWindow *ui;
    void setupUI();
};

#endif // FAVORITEBOOKSWINDOW_H
