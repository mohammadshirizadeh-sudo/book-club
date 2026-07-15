#ifndef PUBLISHERINFODIALOG_H
#define PUBLISHERINFODIALOG_H

#include <QDialog>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class PublisherInfoDialog;
}

class PublisherInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PublisherInfoDialog(NetworkManager* networkManager , QWidget *parent = nullptr);
    ~PublisherInfoDialog();

    void fetchProfileData();


public slots:
    void handleResponse(const Response& response);
private:
    Ui::PublisherInfoDialog *ui;
    NetworkManager* m_networkManager;

};

#endif
