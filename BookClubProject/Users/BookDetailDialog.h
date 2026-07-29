#ifndef BOOKDETAILDIALOG_H
#define BOOKDETAILDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class BookDetailDialog;
}

// Forward declaration so the header doesn't pull in the full GroupReadingWindow
// include chain (QtPdf, QtPdfWidgets, etc.). The .cpp includes the real header.
class GroupReadingWindow;

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

    void on_addCartPushButton_clicked();

    void on_pushButton_clicked();

    void on_groupReadingPushButton_clicked();

private:
    Ui::BookDetailDialog *ui;
    void displayBookInfo(const QVariantMap& bookData);
    NetworkManager* m_networkManager;
    QVariantMap m_bookData;
    bool m_isFavorite;
    bool m_isOwned = false;
    void updateFavoriteButtonAppearance();

    void checkBookOwnership();
    void updateCartButtonAppearance();
    void openBookPdf();

    // Opens the Group Reading page for the book this dialog is currently
    // showing. The dialog is only hidden (not closed) while the Group Reading
    // window is on screen, and is re-shown when the user navigates back from
    // it — mirroring openBookPdf() above.
    //
    // This is self-contained on purpose: previously the button emitted a
    // signal that had to be re-emitted by the parent (UserWindow) and finally
    // wired up in main.cpp, which only happened for the "free books" and
    // "recommended books" entry points. Opening the window directly here
    // makes the button work no matter where the dialog was launched from.
    void openGroupReading();

    // Weak-ish bookkeeping pointer to the Group Reading window opened by
    // this dialog instance. The window is heap-allocated with
    // Qt::WA_DeleteOnClose, so it deletes itself; we only hold the pointer
    // to be able to null it out when the back button is clicked.
    GroupReadingWindow* m_groupReadingWindow = nullptr;
};

#endif // BOOKDETAILDIALOG_H
