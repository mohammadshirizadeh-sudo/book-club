#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(NetworkManager* networkManager , QWidget *parent = nullptr);
    ~InfoDialog();

    void fetchProfileData();


public slots:
    void handleResponse(const Response& response);
private:
    Ui::InfoDialog *ui;
    NetworkManager* m_networkManager;

};

#endif // INFODIALOG_H
