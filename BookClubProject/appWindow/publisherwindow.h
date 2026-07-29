#ifndef PUBLISHERWINDOW_H
#define PUBLISHERWINDOW_H

#include <QWidget>
#include <QVariantMap>

#include "../Network-Manger/NetworkManager.h"

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

class NetworkManager;
class Response;

namespace Ui {
class PublisherWindow;
}

class PublisherWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PublisherWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~PublisherWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_pubProfilePushButton_clicked();
    void on_newBooksPushButton_clicked();
    void on_editBooksPushButton_clicked();
    void on_discountPushButton_clicked();
    void on_deactivatePushButton_clicked();
    void on_notifPushButton_clicked();

    // مدیریت مرکزی پاسخ‌های دریافتی از شبکه
    void handleResponse(const Response& response);

    // اسلات‌های عناصر UI
    void on_refreshStatsButton_clicked();
    void on_chartPeriodCombo_currentIndexChanged(const QString &period);

    void on_quitPushButton_clicked();

    void on_publishedPushButton_clicked();

signals:
    void publisherProfileWindow();
    void myBooksWindow();
    void applydiscountWindow();
    void editWindow();
    void deactivateBook();
    void bookStatisticWindow();
    void notificationWindow();
    void openLoginWindow();
    void publishedBookWindow();
    void signOutRequested();

private:
    Ui::PublisherWindow *ui;
    NetworkManager* m_networkManager;

    // متغیرهای نمودارها - باید قبل از استفاده در setupCharts مقداردهی شوند
    QChart *m_salesTrendChart;
    QChartView *m_salesTrendChartView;

    QChart *m_ratingsChart;
    QChartView *m_ratingsChartView;

    // متدهای کمکی جهت آماده‌سازی اولیه
    void setupCharts();
    void setupTables();

    // متدهای ارسال درخواست به سرور
    void loadAllStatistics();
    void requestSalesOverview();
    void requestSalesTrend();
    void requestBookRatingsChart();
    void requestTopSellingBooks();
    void requestBottomSellingBooks();

    // متدهای بروزرسانی رابط کاربری با داده‌های سرور
    void updateSalesOverviewUI(const QVariantMap& data);
    void updateSalesTrendChartUI(const QVariantMap& data);
    void updateRatingsChartUI(const QVariantMap& data);
    void updateTableUI(class QTableWidget *table, const QVariantList &books);
    qint64 m_pendingLogoutRequestId = -1;

};




#endif // PUBLISHERWINDOW_H