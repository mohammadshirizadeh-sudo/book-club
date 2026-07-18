#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class ChangePasswordDialog;
}

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(NetworkManager* networkManager , QWidget *parent = nullptr);

    ~ChangePasswordDialog();

private slots:
    void on_buttonBox_accepted();

public slots:
    void handleResponse(const Response& response);

private:
    Ui::ChangePasswordDialog *ui;
    NetworkManager* m_networkManager;
};

#endif // CHANGEPASSWORDDIALOG_H