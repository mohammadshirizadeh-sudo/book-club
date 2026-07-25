#ifndef BOOKSTATICSWINDOW_H
#define BOOKSTATICSWINDOW_H

#include <QWidget>
#include <QVariantMap>

// Qt Charts
#include <QtCharts/QChartView>
#include <QtCharts/QChart>

class NetworkManager;
class Response;

namespace Ui {
class BookStaticsWindow;
}

class BookStaticsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BookStaticsWindow(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~BookStaticsWindow();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // مدیریت مرکزی پاسخ‌های دریافتی از شبکه
    void handleResponse(const Response& response);

    // اسلات‌های عناصر UI
    void on_refreshStatsButton_clicked();
    void on_chartPeriodCombo_currentIndexChanged(const QString &period);
    void on_quitPushButton_clicked();

private:
    Ui::BookStaticsWindow *ui;
    NetworkManager *m_networkManager;

    // متغیرهای نمودارها
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
};

#endif // BOOKSTATICSWINDOW_H