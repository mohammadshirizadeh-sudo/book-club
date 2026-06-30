#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
/*
signals:    //اینو واسه وصل کردن صفحه ها نوشتم ولی جواب نمیده، مثل 2 روش دیگه ای که امتحان کردم
    void openRegisterWindow();
*/
private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
