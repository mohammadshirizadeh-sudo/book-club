#ifndef SHELFMANAGEMENTDIALOG_H
#define SHELFMANAGEMENTDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QVariantMap>
#include <QMap>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Response.h"

namespace Ui {
class ShelfManagementDialog;
}

class ShelfManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShelfManagementDialog(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~ShelfManagementDialog();

private slots:
    // مدیریت پاسخ‌های دریافتی از سرور
    void handleResponse(const Response& response);

    // اسلات‌های مربوط به سیگنال‌های ui
    void on_addShelfButton_clicked();
    void on_deleteShelfButton_clicked();
    void on_renameShelfButton_clicked();
    void on_removeBookButton_clicked();
    void on_moveBookButton_clicked();
    void on_readPdfButton_clicked();
    void on_shelfList_currentRowChanged(int currentRow);
    void on_bookList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);



protected:
    void showEvent(QShowEvent *event) override;
private:
    Ui::ShelfManagementDialog *ui;
    NetworkManager *m_networkManager;

    int m_currentShelfId = -1;
    QVariantMap m_currentBookData;

    // نگه‌داری نگاشت نام قفسه به ID برای استفاده در جابه‌جایی (Move)
    QMap<QString, int> m_shelvesMap;

    // توابع کمکی
    void loadShelves();
    void loadBooksInShelf(int shelfId);
    void updateBookDetails(const QVariantMap& bookData);
    void clearBookDetails();
};

#endif // SHELFMANAGEMENTDIALOG_H