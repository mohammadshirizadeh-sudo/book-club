#ifndef PUBLISHEDBOOKSWINDOW_H
#define PUBLISHEDBOOKSWINDOW_H

#include <QWidget>
#include <QMap>
#include <QVariantMap>
#include <QListWidgetItem>
#include <QShowEvent>
#include "../Network-Manger/NetworkManager.h"

namespace Ui {
class PublishedBooksWindow;
}

class Response;

class PublishedBooksWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublishedBooksWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~PublishedBooksWindow();

    // متد برای ارسال درخواست مجدد دریافت کتاب‌ها در صورت نیاز
    void fetchPublisherBooks();

signals:
    void backRequested(); // سیگنال بازگشت به صفحه قبل (یا می‌توانید نام آن را userWindow بذارید)

private slots:
    // مدیریت پاسخ‌های شبکه (دریافت لیست کتاب‌ها و کاورها)
    void handleResponse(const Response& response);

    // اسلات کلیک روی آیتم‌های لیست کتاب‌ها
    void on_publishedListWidget_itemClicked(QListWidgetItem *item);

    // اسلات دکمه بازگشت
    void on_backPushButton_clicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::PublishedBooksWindow *ui;
    NetworkManager* m_networkManager;
    int m_publisherId;

    // کش اطلاعات کتاب‌ها جهت ارسال به BookDetailDialog
    QMap<int, QVariantMap> m_booksCache;
};

#endif // PUBLISHEDBOOKSWINDOW_H