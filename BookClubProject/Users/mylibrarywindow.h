#ifndef MYLIBRARYWINDOW_H
#define MYLIBRARYWINDOW_H

#include <QWidget>

namespace Ui {
class MyLibraryWindow;
}

class MyLibraryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MyLibraryWindow(QWidget *parent = nullptr);
    ~MyLibraryWindow();

private:
    Ui::MyLibraryWindow *ui;
};

#endif // MYLIBRARYWINDOW_H
