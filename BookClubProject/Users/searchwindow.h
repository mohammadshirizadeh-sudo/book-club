#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QWidget>
#include <QMap>
#include <QVariant>
#include <QListWidgetItem>
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


    // اسلاتی که به سیگنال شبکه متصل شده است
    void handleResponse(const Response& response);

    void on_showSearchPushButton_clicked();

    void on_searchResultsListWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::SearchWindow *ui;
    NetworkManager* m_networkManager;

    // کش مربوط به ذخیره‌سازی کتاب‌های جستجو شده (شناسه کتاب -> اطلاعات کتاب)
    QMap<int, QVariantMap> m_searchBooksCache;
};

#endif // SEARCHWINDOW_H