#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QWidget>
#include <QMap>
#include <QVariant>
#include "../Network-Manger/NetworkManager.h"


namespace Ui {
class SearchWindow;
}
class NetworkManager;
class Response;

class SearchWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~SearchWindow();

private slots:
    // این اسلات به صورت خودکار توسط Qt به دکمه متصل می‌شود (Auto-connection)
    void on_searchBookPushButton_clicked();

    // اسلاتی که به سیگنال شبکه متصل شده است
    void handleResponse(const Response& response);

private:
    Ui::SearchWindow *ui;
    NetworkManager* m_networkManager;

    // کش مربوط به ذخیره‌سازی کتاب‌های جستجو شده (شناسه کتاب -> اطلاعات کتاب)
    QMap<int, QVariantMap> m_searchBooksCache;
};

#endif // SEARCHWINDOW_H