#ifndef ADDNEWBOOKDIALOG_H
#define ADDNEWBOOKDIALOG_H

#include "../Network-Manger/NetworkManager.h"
#include <QDialog>

namespace Ui {
class AddNewBookDialog;
}

class AddNewBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddNewBookDialog(NetworkManager* networkManager,QWidget *parent = nullptr);
    ~AddNewBookDialog();

private slots:
    void on_addPictureButton_clicked();
    // void on_okButton_clicked();
    void handleResponse(const Response& response);

    void on_buttonBox_accepted();

private:
    Ui::AddNewBookDialog *ui;
    QString m_savedCoverPath;
    NetworkManager* m_networkManager;
    QByteArray m_imageBytes;
};

#endif // ADDNEWBOOKDIALOG_H
