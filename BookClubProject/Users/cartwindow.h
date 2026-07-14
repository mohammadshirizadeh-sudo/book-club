#ifndef CARTWINDOW_H
#define CARTWINDOW_H

#include <QWidget>

namespace Ui {
class CartWindow;
}

class CartWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CartWindow(QWidget *parent = nullptr);
    ~CartWindow();

private:
    Ui::CartWindow *ui;
};

#endif // CARTWINDOW_H
