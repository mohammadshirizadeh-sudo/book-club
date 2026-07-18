#ifndef AUTHORDETAILDIALOG_H
#define AUTHORDETAILDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include <QMap>
#include <QListWidgetItem>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class AuthorDetailDialog;
}

class AuthorDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthorDetailDialog(const QVariantMap& authorData,NetworkManager* networkManager, QWidget *parent = nullptr);
    ~AuthorDetailDialog();

private slots:
    // اسلات کلیک روی کتاب‌های نویسنده
    void on_booksListWidget_itemClicked(QListWidgetItem *item);
    void onResponseReceived(const Response& response);

private:
    Ui::AuthorDetailDialog *ui;
    void displayAuthorInfo(const QVariantMap& authorData);

    // کش محلی برای ذخیره موقت اطلاعات کتاب‌های این نویسنده
    QMap<int, QVariantMap> m_authorBooksCache;
    NetworkManager* m_networkManager;
};

#endif // AUTHORDETAILDIALOG_H