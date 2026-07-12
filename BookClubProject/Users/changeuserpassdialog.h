#ifndef CHANGEUSERPASSDIALOG_H
#define CHANGEUSERPASSDIALOG_H


#include <QDialog>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class ChangeUserPassDialog;
}

class ChangeUserPassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserPassDialog(NetworkManager* networkManager,QWidget *parent = nullptr);
    ~ChangeUserPassDialog();

private slots:
    void on_buttonBox_accepted();


public slots:
    void handleResponse(const Response& response);
private:
    Ui::ChangeUserPassDialog *ui;
    NetworkManager* m_networkManager;
};

#endif // CHANGEUSERPASSDIALOG_H
