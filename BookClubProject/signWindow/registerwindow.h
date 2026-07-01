#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

namespace Ui {
class RegisterWindow;
}

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void openMainWindow();
    void openLoginWindow();

private slots:
    void on_signupPushButton_clicked();
    void on_backToSigninPushButton_clicked();


private:
    Ui::RegisterWindow *ui;
};

#endif // REGISTERWINDOW_H
