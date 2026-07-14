#ifndef EDITINFODIALOG_H
#define EDITINFODIALOG_H

#include <QDialog>
#include "../Server/Response.h"
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class EditInfoDialog;
}

class EditInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditInfoDialog(NetworkManager* networkManager,QWidget *parent = nullptr);
    ~EditInfoDialog();

private slots:
    void on_buttonBox_accepted();


public slots:
    void handleResponse(const Response& response);

private:
    Ui::EditInfoDialog *ui;
     NetworkManager* m_networkManager;
};

#endif
