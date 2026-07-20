#ifndef BOOKSTATICSWINDOW_H
#define BOOKSTATICSWINDOW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>


QT_BEGIN_NAMESPACE
namespace Ui { class BookStaticsWindow; }
QT_END_NAMESPACE

class BookStaticsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BookStaticsWindow(QWidget *parent = nullptr);
    ~BookStaticsWindow();

private slots:
    void on_refreshStatsButton_clicked();
    void on_chartPeriodCombo_currentIndexChanged(int index);

private:
    Ui::BookStaticsWindow *ui;

    // Chart widgets
    QChartView *m_salesChartView;
    QChartView *m_ratingsChartView;
    QChart *m_salesChart;
    QChart *m_ratingsChart;

    // Data methods
    void initializeCharts();
    void loadStatisticsData();
    void updateSalesChart(const QString &period);
    void updateRatingsChart();
    void populateBestSellersTable();
    void populateWorstSellersTable();
    void updateSummaryCards();

    // Data access (to be connected to data manager)
    double getTotalRevenue() const;
    int getTotalBooksCount() const;
    double getAverageRating() const;
    int getTotalSalesCount() const;
};

#endif // BOOKSTATICSWINDOW_H