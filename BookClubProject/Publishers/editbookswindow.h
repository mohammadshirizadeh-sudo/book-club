#ifndef EDITBOOKSWINDOW_H
#define EDITBOOKSWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include <QVariantMap>
#include <QList>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Response.h"

namespace Ui {
class EditBooksWindow;
}

class EditBooksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EditBooksWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~EditBooksWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // مدیریت مرکزی پاسخ‌های شبکه
    void handleResponse(const Response& response);

    // اسلات‌های دکمه‌ها و فرم
    void on_refreshListBtn_clicked();
    void on_searchBookLineEdit_textChanged(const QString &arg1);
    void on_booksListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_uploadCoverButton_clicked();
    void on_removeCoverButton_clicked();
    void on_uploadPdfButton_clicked();

    void on_saveChangesButton_clicked();
    void on_resetFormButton_clicked();
    void on_quitPushButton_clicked();

private:
    Ui::EditBooksWindow *ui;
    NetworkManager *m_networkManager;

    int m_selectedBookId = -1;
    QVariantMap m_currentBookData;

    QString m_selectedCoverPath;
    QString m_selectedPdfPath;

    // توابع کمکی
    void loadPublisherBooks();
    void loadBookDetails(int bookId);
    void populateForm(const QVariantMap& bookData);
    void clearForm();
    void setFormEnabled(bool enabled);
    void updateCoverPreview(const QString& coverPath);
};

#endif // EDITBOOKSWINDOW_H