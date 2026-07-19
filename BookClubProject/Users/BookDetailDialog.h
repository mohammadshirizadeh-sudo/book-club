#ifndef BOOKDETAILDIALOG_H
#define BOOKDETAILDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class BookDetailDialog;
}

class BookDetailDialog : public QDialog
{
    Q_OBJECT

public:
    // سازنده را طوری تغییر می‌دهیم که اطلاعات کتاب (bookData) را ورودی بگیرد
    explicit BookDetailDialog(NetworkManager* networkManager , const QVariantMap& bookData, QWidget *parent = nullptr);
    ~BookDetailDialog();

private slots:
    void on_addFavoritePushButton_clicked();
    void onResponseReceived(const Response& response);

private:
    Ui::BookDetailDialog *ui;
    void displayBookInfo(const QVariantMap& bookData);
    NetworkManager* m_networkManager;
    QVariantMap m_bookData;
    bool m_isFavorite;
    void updateFavoriteButtonAppearance();
};

#endif // BOOKDETAILDIALOG_H