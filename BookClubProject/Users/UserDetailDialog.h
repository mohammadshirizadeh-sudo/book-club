#ifndef USERDETAILDIALOG_H
#define USERDETAILDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include<QListWidgetItem>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class UserDetailDialog;
}

class UserDetailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserDetailDialog(const QVariantMap& userData, QWidget *parent = nullptr);
    ~UserDetailDialog();



private slots:
    void on_booksListWidget_itemClicked(QListWidgetItem *item);
private:
    Ui::UserDetailDialog *ui;
    void displayUserInfo(const QVariantMap& userData);

    QMap<int, QVariantMap> m_publisherBooksCache;
    NetworkManager* m_networkManager;
};
#endif