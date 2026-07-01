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

signals:
    void openForgotPasswordWindow();
    void openRegisterWindow();
    void openMainWindow();

private slots:
    void on_forgotpassPushButton_clicked();
    void on_registerPushButton_clicked();
    void on_signinPushButton_clicked();

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
