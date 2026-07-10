#ifndef USERPROFILEWINDOW_H
#define USERPROFILEWINDOW_H

#include <QWidget>

namespace Ui {
class UserProfileWindow;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

signals:
    void openGenreWindow();
    void openUserInfoDialog();
    void openEditUserInfoDialog();
    void openChangeUserPassDialog();
    void openShoppingHistoryDialog();
    void openFavBooksDialog();

private slots:
    void on_favGenresPushButton_clicked();
    void on_UserInfPushButton_clicked();
    void on_editUserInfPushButton_clicked();
    void on_changePassPushButton_clicked();
    void on_shoppingHistoryPushButton_clicked();
    void on_favBooksPushButton_clicked();

private:
    Ui::UserProfileWindow *ui;
};

#endif // USERPROFILEWINDOW_H
