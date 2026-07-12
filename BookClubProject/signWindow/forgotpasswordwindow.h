#ifndef FORGOTPASSWORDWINDOW_H
#define FORGOTPASSWORDWINDOW_H
#include "../Network-Manger/NetworkManager.h"

#include <QWidget>

namespace Ui {
class ForgotPasswordWindow;
}

class ForgotPasswordWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ForgotPasswordWindow(NetworkManager* networkManager  , QWidget *parent = nullptr);
    ~ForgotPasswordWindow();




signals:
    void openLoginWindow(); // برای بازگشت به صفحه لاگین

    void openUserWindow();

private slots:    
    void on_tokenPushButton_clicked();

    void on_remmemberPushButton_clicked();

public slots:


    void handleResponse(const Response& response);

private:
    Ui::ForgotPasswordWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // FORGOTPASSWORDWINDOW_H
