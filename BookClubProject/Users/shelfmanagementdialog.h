#ifndef SHELFMANAGEMENTDIALOG_H
#define SHELFMANAGEMENTDIALOG_H

#include <QDialog>
#include "../NetworkManger/NetworkManager.h"
#include<QListWidgetItem>


namespace Ui {
class ShelfManagementDialog;
}

class ShelfManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShelfManagementDialog(NetworkManager* networkManager, int userId, QWidget *parent = nullptr);
    ~ShelfManagementDialog();

    void loadShelves();

signals:
    void openPdfReader(int bookId);
    void openBookDetail(int bookId);

private slots:
    void on_addShelfButton_clicked();
    void on_deleteShelfButton_clicked();
    void on_renameShelfButton_clicked();
    void on_removeBookButton_clicked();
    void on_moveBookButton_clicked();
    void on_readPdfButton_clicked();
    void on_shelfList_currentRowChanged(int currentRow);
    void on_bookList_currentItemChanged(QListWidgetItem *current);
    void onResponseReceived(const Response& response);

    // void on_backButton_clicked();

private:
    Ui::ShelfManagementDialog *ui;
    NetworkManager* m_networkManager;
    int m_userId;

    int m_currentShelfId;
    int m_currentBookId;

    void loadBooksForShelf(int shelfId);
    void updateBookDetails(int bookId);
    void clearBookDetails();

    int getSelectedShelfId() const;
    int getSelectedBookId() const;
};

#endif // SHELFMANAGEMENTDIALOG_H
