#ifndef DEACTIVATEBOOKWINDOW_H
#define DEACTIVATEBOOKWINDOW_H

#include <QWidget>
#include <QVariantMap>
#include <QList>
#include <QTableWidgetItem>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Response.h"

namespace Ui {
class DeactivateBookWindow;
}

class DeactivateBookWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DeactivateBookWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~DeactivateBookWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // مدیریت مرکزی پاسخ‌های شبکه
    void handleResponse(const Response& response);

    // اسلات‌های تغییرات فیلتر و جستجو
    void on_searchLineEdit_textChanged(const QString &text);
    void on_allBooksRadio_toggled(bool checked);
    void on_activeBooksRadio_toggled(bool checked);
    void on_deactivatedBooksRadio_toggled(bool checked);
    void on_refreshListButton_clicked();
    void on_exportListButton_clicked();

    // اسلات‌های جدول و عملیات روی کتاب
    void on_booksTable_itemSelectionChanged();
    void on_deactivateBookButton_clicked();
    void on_reactivateBookButton_clicked();
    void on_viewDetailsButton_clicked();
    void on_backPushButton_clicked();

private:
    Ui::DeactivateBookWindow *ui;
    NetworkManager *m_networkManager;

    QList<QVariantMap> m_allBooks;
    int m_selectedBookId = -1;
    QVariantMap m_selectedBookData;

    // توابع کمکی
    void loadPublisherBooks();
    void populateTable();
    void updateSelectedBookUI();
    void exportToCSV();
};

#endif // DEACTIVATEBOOKWINDOW_H