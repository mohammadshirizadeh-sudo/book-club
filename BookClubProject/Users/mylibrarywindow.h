#ifndef MYLIBRARYWINDOW_H
#define MYLIBRARYWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MyLibraryWindow; }
QT_END_NAMESPACE

class MyLibraryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MyLibraryWindow(QWidget *parent = nullptr);
    ~MyLibraryWindow();

private slots:
    void on_backButton_clicked();

private:
    Ui::MyLibraryWindow *ui;
    void setupUI();
};

#endif // MYLIBRARYWINDOW_H
