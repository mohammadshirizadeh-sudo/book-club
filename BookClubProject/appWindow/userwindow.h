#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Book.h"

namespace Ui {
class UserWindow;
}

class UserWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~UserWindow();

    void loadFreeBooks();
    void loadRecommendedBooks();

    void loadNewBooks();

private slots:
    void on_freeBooksListWidget_itemClicked(QListWidgetItem *item);
    void handleResponse(const Response& response);

    void on_nextPushButton_clicked();

    void on_prevPushButton_clicked();



    void on_recommendedBooksListWidget_itemClicked(QListWidgetItem *item);
    void on_nextRecPushButton_clicked();
    void on_prevRecPushButton_clicked();


    //for recently books added

    void on_newBooksListWidget_itemClicked(QListWidgetItem *item);
    void on_nextNewPushButton_clicked();
    void on_prevNewPushButton_clicked();

    // void on_pushButton_2_clicked();

    void on_pushButton_2_clicked();



signals:
    void userProfileWindow();

private:
    Ui::UserWindow *ui;
    NetworkManager* m_networkManager;
    QMap<int, QVariantMap> m_booksCache;

    QVariantList m_allFreeBooks; // نگهداری کل کتاب‌های دریافتی
    int m_currentPage = 0;       // صفحه‌ی فعلی (شروع از 0)
    int m_booksPerPage = 1;


    QVariantList m_allRecBooks;
    int m_currentRecPage = 0;
    int m_recBooksPerPage = 2; // 👈 تنظیم روی ۲ کتاب در هر صفحه
    QMap<int, QVariantMap> m_recBooksCache;


    void updateBooksDisplay();

    void updateRecommendedBooksDisplay();



    //for recently added books
    QVariantList m_allNewBooks;
    int m_currentNewPage = 0;
    int m_newBooksPerPage = 2; // تعداد کتاب در هر صفحه (می‌توانید روی 2 یا هر تعداد دیگری تنظیم کنید)
    QMap<int, QVariantMap> m_newBooksCache;
    void updateNewBooksDisplay();
};

#endif // USERWINDOW_H