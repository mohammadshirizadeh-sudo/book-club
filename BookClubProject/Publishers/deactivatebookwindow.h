#ifndef DEACTIVATEBOOKWINDOW_H
#define DEACTIVATEBOOKWINDOW_H

#include <QWidget>

namespace Ui {
class DeactivateBookWindow;
}

class DeactivateBookWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DeactivateBookWindow(QWidget *parent = nullptr);
    ~DeactivateBookWindow();

signals:
    void showHomePageRequested();
    void showProfileRequested();
    void showMyBooksRequested();
    void signOutRequested();
    void showAddNewBookRequested();
    void showEditBooksRequested();
    void showBookStaticsRequested();
    void showApplyDiscountRequested();
    void showNotificationsRequested();

private slots:
    // Functionality slots
    void on_allBooksRadio_toggled(bool checked);
    void on_activeBooksRadio_toggled(bool checked);
    void on_deactivatedBooksRadio_toggled(bool checked);
    void on_searchLineEdit_textChanged(const QString &text);
    void on_refreshListButton_clicked();
    void on_exportListButton_clicked();
    void on_booksTable_cellClicked(int row, int column);
    void on_booksTable_cellDoubleClicked(int row, int column);
    void on_deactivateBookButton_clicked();
    void on_reactivateBookButton_clicked();
    void on_deleteBookButton_clicked();
    void on_viewDetailsButton_clicked();

private:
    Ui::DeactivateBookWindow *ui;

    // Current selection state
    int m_selectedBookId;
    QString m_selectedBookTitle;
    bool m_isSelectedBookActive;

    // Data methods
    void loadBooksList(const QString &filter = "all", const QString &searchText = "");
    void updateSelectionState(int row);
    void updateActionButtons();
    void deactivateSelectedBook();
    void reactivateSelectedBook();
    void deleteSelectedBook();
    void viewBookDetails();
    void exportToCSV();
    QString getCurrentFilter() const;

    // Database methods (to be connected)
    struct BookData {
        int id;
        QString title;
        QString author;
        double price;
        QString status;  // "active" or "deactivated"
        int sales;
        QString publishDate;
        QString coverPath;
    };

    QList<BookData> getPublisherBooksFromDB() const;
    QList<BookData> filterBooks(const QList<BookData> &books,
                                const QString &statusFilter,
                                const QString &searchText) const;
    bool deactivateBookInDatabase(int bookId);
    bool reactivateBookInDatabase(int bookId);
    bool deleteBookPermanently(int bookId);
};

#endif // DEACTIVATEBOOKWINDOW_H
