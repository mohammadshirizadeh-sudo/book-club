#ifndef FORGOTPASSWORDWINDOW_H
#define FORGOTPASSWORDWINDOW_H

#include <QWidget>

namespace Ui {
class ForgotPasswordWindow;
}

class ForgotPasswordWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ForgotPasswordWindow(QWidget *parent = nullptr);
    ~ForgotPasswordWindow();

signals:
    void openMainWindow();

private slots:
    void on_remmemberPushButton_clicked();

private:
    Ui::ForgotPasswordWindow *ui;
};

#endif // FORGOTPASSWORDWINDOW_H
