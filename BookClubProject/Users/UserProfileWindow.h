#ifndef USERPROFILEWINDOW_H
#define USERPROFILEWINDOW_H

#include <QWidget>

#include "../Network-Manger/NetworkManager.h"


namespace Ui {
class UserProfileWindow;
}

class UserProfileWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserProfileWindow(NetworkManager* networkManager  ,QWidget *parent = nullptr);
    ~UserProfileWindow();


    void loadprof();


signals:
    void openGenreWindow();
    void openUserInfoDialog();
    void openEditUserInfoDialog();
    void openChangeUserPassDialog();
    void openShoppingHistoryDialog();
    void openFavBooksWindow();

private slots:
    void on_favGenresPushButton_clicked();
    void on_UserInfPushButton_clicked();
    void on_editUserInfPushButton_clicked();
    void on_changePassPushButton_clicked();
    void on_shoppingHistoryPushButton_clicked();
    void on_favBooksPushButton_clicked();


    void handleResponse(const Response& response);

private:
    Ui::UserProfileWindow *ui;
    NetworkManager* m_networkManager;
};

#endif // USERPROFILEWINDOW_H
