#ifndef FAVORITEBOOKSWINDOW_H
#define FAVORITEBOOKSWINDOW_H

#include <QWidget>

namespace Ui {
class FavoriteBooksWindow;
}

class FavoriteBooksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FavoriteBooksWindow(QWidget *parent = nullptr);
    ~FavoriteBooksWindow();

private:
    Ui::FavoriteBooksWindow *ui;
};

#endif // FAVORITEBOOKSWINDOW_H
