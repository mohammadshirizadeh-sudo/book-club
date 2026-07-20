#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Book.h"
#include <QLabel>

namespace Ui {
class UserWindow;
}

class UserWindow : public QWidget
{
    Q_OBJECT


protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    explicit UserWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~UserWindow();

    void loadFreeBooks();
    void loadRecommendedBooks();

    void loadNewBooks();
    void loadBestSellers();
    void loadCoverInto(QLabel* label,int bookId,QSize targetSize);

private slots:
    // void on_freeBooksListWidget_itemClicked(QListWidgetItem *item);
    void handleResponse(const Response& response);

    void on_nextPushButton_clicked();

    void on_prevPushButton_clicked();



    // void on_recommendedBooksListWidget_itemClicked(QListWidgetItem *item);
    void on_nextRecPushButton_clicked();
    void on_prevRecPushButton_clicked();


    //for recently books added

    // void on_newBooksListWidget_itemClicked(QListWidgetItem *item);
    void on_nextNewPushButton_clicked();
    void on_prevNewPushButton_clicked();

    // void on_pushButton_2_clicked();

    void on_pushButton_2_clicked();



    void on_pushButton_5_clicked();



    // void on_bestSellersListWidget_itemClicked(QListWidgetItem *item);

    void on_nextBestSellerPushButton_clicked();
    void on_prevBestSellerPushButton_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

signals:
    void userProfileWindow();
    void searchWindow();
    void genrebrowsWindow();
    void cartWindow();
    void libraryWindow();

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


    QVariantList m_allBestSellers;
    int m_currentBestSellerPage = 0;
    int m_bestSellersPerPage = 1;
    QMap<int, QVariantMap> m_bestSellersCache;
    void updateBestSellersDisplay();


    void updateBooksDisplay();

    void updateRecommendedBooksDisplay();



    //for recently added books
    QVariantList m_allNewBooks;
    int m_currentNewPage = 0;
    int m_newBooksPerPage = 2; // تعداد کتاب در هر صفحه (می‌توانید روی 2 یا هر تعداد دیگری تنظیم کنید)
    QMap<int, QVariantMap> m_newBooksCache;
    void updateNewBooksDisplay();


    void onFreeBookClicked();
    void onRecommendedBookClicked(int offset);
    void onNewBookClicked();

    void onBestSellerClicked();



    QMap<int, QPixmap> m_coverCache;

    QMultiMap<int, QPointer<QLabel>> m_pendingCoverLabels;
};

#endif // USERWINDOW_H