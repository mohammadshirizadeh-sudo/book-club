

/*
#include "bookstaticswindow.h"
#include "Publishers/ui_bookstaticswindow.h"
#include <QDebug>
#include <QMessageBox>

BookStaticsWindow::BookStaticsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BookStaticsWindow),
    m_salesChartView(nullptr),
    m_ratingsChartView(nullptr),
    m_salesChart(nullptr),
    m_ratingsChart(nullptr)
{
    ui->setupUi(this);

    // Initialize charts
    initializeCharts();

    // Load initial data
    loadStatisticsData();
}

BookStaticsWindow::~BookStaticsWindow()
{
    delete ui;
}

void BookStaticsWindow::initializeCharts()
{
    // === Sales Trend Chart (Line Chart) ===
    m_salesChart = new QChart();
    m_salesChart->setTitle("Sales Over Time");
    m_salesChart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    m_salesChart->setAnimationOptions(QChart::SeriesAnimations);
    m_salesChart->legend()->setVisible(true);
    m_salesChart->legend()->setAlignment(Qt::AlignBottom);
    m_salesChart->setBackgroundVisible(false);

    // Create line series for sales data
    QLineSeries *salesSeries = new QLineSeries();
    salesSeries->setName("Sales Count");
    salesSeries->setColor(QColor(0, 85, 170));
    salesSeries->setPen(QPen(QColor(0, 85, 170), 3));

    // Sample data - will be replaced with real data
    QStringList categories;
    for (int i = 1; i <= 7; i++) {
        categories << QString("Day %1").arg(i);
        *salesSeries << QPointF(i, qrand() % 50 + 10);
    }

    m_salesChart->addSeries(salesSeries);

    // Configure axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    m_salesChart->addAxis(axisX, Qt::AlignBottom);
    salesSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 60);
    axisY->setTitleText("Sales");
    axisY->setTitleFont(QFont("Segoe UI", 10));
    m_salesChart->addAxis(axisY, Qt::AlignLeft);
    salesSeries->attachAxis(axisY);

    // Create chart view and add to placeholder
    m_salesChartView = new QChartView(m_salesChart, ui->salesChartPlaceholder);
    m_salesChartView->setRenderHint(QPainter::Antialiasing);
    m_salesChartView->setGeometry(ui->salesChartPlaceholder->geometry());
    m_salesChartView->show();

    // === Ratings Bar Chart ===
    m_ratingsChart = new QChart();
    m_ratingsChart->setTitle("Average Rating by Book");
    m_ratingsChart->setTitleFont(QFont("Segoe UI", 12, QFont::Bold));
    m_ratingsChart->setAnimationOptions(QChart::SeriesAnimations);
    m_ratingsChart->legend()->setVisible(false);
    m_ratingsChart->setBackgroundVisible(false);

    // Create bar series for ratings
    QBarSeries *barSeries = new QBarSeries();

    // Sample rating data
    QBarSet *ratingsSet = new QBarSet("Rating");
    ratingsSet->setColor(QColor(255, 152, 0));
    ratingsSet->setLabelColor(QColor(50, 50, 50));

    // Sample books with ratings
    QStringList bookNames;
    bookNames << "Book A" << "Book B" << "Book C" << "Book D" << "Book E";

    double sampleRatings[] = {4.5, 3.8, 4.2, 3.5, 4.8};
    for (int i = 0; i < 5; i++) {
        *ratingsSet << sampleRatings[i];
    }
    barSeries->append(ratingsSet);
    m_ratingsChart->addSeries(barSeries);

    // Configure axes for bar chart
    QBarCategoryAxis *ratingAxisX = new QBarCategoryAxis();
    ratingAxisX->append(bookNames);
    m_ratingsChart->addAxis(ratingAxisX, Qt::AlignBottom);
    barSeries->attachAxis(ratingAxisX);

    QValueAxis *ratingAxisY = new QValueAxis();
    ratingAxisY->setRange(0, 5);
    ratingAxisY->setTitleText("Stars (1-5)");
    ratingAxisY->setTitleFont(QFont("Segoe UI", 10));
    ratingAxisY->setTickCount(6);
    m_ratingsChart->addAxis(ratingAxisY, Qt::AlignLeft);
    barSeries->attachAxis(ratingAxisY);

    // Create chart view and add to placeholder
    m_ratingsChartView = new QChartView(m_ratingsChart, ui->ratingsChartPlaceholder);
    m_ratingsChartView->setRenderHint(QPainter::Antialiasing);
    m_ratingsChartView->setGeometry(ui->ratingsChartPlaceholder->geometry());
    m_ratingsChartView->show();

    // Connect resize events to update chart positions
    connect(ui->salesChartPlaceholder, &QWidget::resized, this, [this]() {
        if (m_salesChartView) {
            m_salesChartView->setGeometry(ui->salesChartPlaceholder->geometry());
        }
    });

    connect(ui->ratingsChartPlaceholder, &QWidget::resized, this, [this]() {
        if (m_ratingsChartView) {
            m_ratingsChartView->setGeometry(ui->ratingsChartPlaceholder->geometry());
        }
    });
}

void BookStaticsWindow::loadStatisticsData()
{
    updateSummaryCards();
    populateBestSellersTable();
    populateWorstSellersTable();
    updateRatingsChart();
    updateSalesChart("Daily");
}

void BookStaticsWindow::updateSummaryCards()
{
    // Update summary cards with data
    ui->revenueValueLabel->setText(QString("$%1").arg(getTotalRevenue(), 0, 'f', 2));
    ui->booksValueLabel->setText(QString::number(getTotalBooksCount()));
    ui->ratingValueLabel->setText(QString("⭐ %1").arg(getAverageRating(), 0, 'f', 1));
    ui->salesValueLabel->setText(QString::number(getTotalSalesCount()));
}

void BookStaticsWindow::updateSalesChart(const QString &period)
{
    if (!m_salesChart) return;

    // Clear existing series
    m_salesChart->removeAllSeries();

    // Remove old axes
    QList<QAbstractAxis*> axes = m_salesChart->axes();
    for (QAbstractAxis* axis : axes) {
        m_salesChart->removeAxis(axis);
    }

    // Create new series
    QLineSeries *salesSeries = new QLineSeries();
    salesSeries->setName("Sales Count");
    salesSeries->setColor(QColor(0, 85, 170));
    salesSeries->setPen(QPen(QColor(0, 85, 170), 3));

    QStringList categories;
    int dataPoints = 7;

    if (period == "Weekly") {
        dataPoints = 4;
        for (int i = 1; i <= dataPoints; i++) {
            categories << QString("Week %1").arg(i);
            *salesSeries << QPointF(i, qrand() % 200 + 50);
        }
    } else if (period == "Monthly") {
        dataPoints = 6;
        for (int i = 1; i <= dataPoints; i++) {
            categories << QString("Month %1").arg(i);
            *salesSeries << QPointF(i, qrand() % 500 + 100);
        }
    } else { // Daily
        for (int i = 1; i <= dataPoints; i++) {
            categories << QString("Day %1").arg(i);
            *salesSeries << QPointF(i, qrand() % 50 + 10);
        }
    }

    m_salesChart->addSeries(salesSeries);

    // Reconfigure axes
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    m_salesChart->addAxis(axisX, Qt::AlignBottom);
    salesSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    int maxY = (period == "Monthly") ? 600 : (period == "Weekly") ? 250 : 60;
    axisY->setRange(0, maxY);
    axisY->setTitleText("Sales");
    axisY->setTitleFont(QFont("Segoe UI", 10));
    m_salesChart->addAxis(axisY, Qt::AlignLeft);
    salesSeries->attachAxis(axisY);
}

void BookStaticsWindow::updateRatingsChart()
{
    if (!m_ratingsChart) return;

    // This would be updated with real data from database
    // For now using sample data set in initializeCharts
}

void BookStaticsWindow::populateBestSellersTable()
{
    ui->bestSellersTable->setRowCount(5);

    // Sample best sellers data
    struct BestSellerData {
        QString title;
        int sales;
        double revenue;
    };

    BestSellerData sampleData[] = {
        {"The Great Adventure", 150, 449.99},
        {"Mystery of Shadows", 132, 395.99},
        {"Science Today", 98, 294.00},
        {"Poetry Collection", 87, 261.00},
        {"History Revealed", 76, 228.00}
    };

    for (int row = 0; row < 5; row++) {
        QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(row + 1));
        rankItem->setTextAlignment(Qt::AlignCenter);
        rankItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        rankItem->setBackground(QColor(232, 245, 233));
        ui->bestSellersTable->setItem(row, 0, rankItem);

        QTableWidgetItem *titleItem = new QTableWidgetItem(sampleData[row].title);
        titleItem->setFont(QFont("Segoe UI", 10));
        ui->bestSellersTable->setItem(row, 1, titleItem);

        QTableWidgetItem *salesItem = new QTableWidgetItem(QString::number(sampleData[row].sales));
        salesItem->setTextAlignment(Qt::AlignCenter);
        salesItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->bestSellersTable->setItem(row, 2, salesItem);

        QTableWidgetItem *revenueItem = new QTableWidgetItem(QString("$%1").arg(sampleData[row].revenue, 0, 'f', 2));
        revenueItem->setTextAlignment(Qt::AlignCenter);
        revenueItem->setForeground(QColor(56, 142, 60));
        ui->bestSellersTable->setItem(row, 3, revenueItem);
    }
}

void BookStaticsWindow::populateWorstSellersTable()
{
    ui->worstSellersTable->setRowCount(5);

    // Sample worst sellers data
    struct WorstSellerData {
        QString title;
        int sales;
        double revenue;
    };

    WorstSellerData sampleData[] = {
        {"Advanced Mathematics", 8, 24.00},
        {"Rare Botany Guide", 12, 36.00},
        {"Local History Vol.3", 15, 45.00},
        {"Niche Philosophy", 18, 54.00},
        {"Technical Manual", 22, 66.00}
    };

    for (int row = 0; row < 5; row++) {
        QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(row + 1));
        rankItem->setTextAlignment(Qt::AlignCenter);
        rankItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        rankItem->setBackground(QColor(255, 235, 238));
        ui->worstSellersTable->setItem(row, 0, rankItem);

        QTableWidgetItem *titleItem = new QTableWidgetItem(sampleData[row].title);
        titleItem->setFont(QFont("Segoe UI", 10));
        ui->worstSellersTable->setItem(row, 1, titleItem);

        QTableWidgetItem *salesItem = new QTableWidgetItem(QString::number(sampleData[row].sales));
        salesItem->setTextAlignment(Qt::AlignCenter);
        salesItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->worstSellersTable->setItem(row, 2, salesItem);

        QTableWidgetItem *revenueItem = new QTableWidgetItem(QString("$%1").arg(sampleData[row].revenue, 0, 'f', 2));
        revenueItem->setTextAlignment(Qt::AlignCenter);
        revenueItem->setForeground(QColor(198, 40, 40));
        ui->worstSellersTable->setItem(row, 3, revenueItem);
    }
}

// ==================== Data Access Methods (To be connected to real data source) ====================

double BookStaticsWindow::getTotalRevenue() const
{
    // TODO: Connect to actual data source
    return 1629.97;
}

int BookStaticsWindow::getTotalBooksCount() const
{
    // TODO: Connect to actual data source
    return 15;
}

double BookStaticsWindow::getAverageRating() const
{
    // TODO: Connect to actual data source
    return 4.16;
}

int BookStaticsWindow::getTotalSalesCount() const
{
    // TODO: Connect to actual data source
    return 561;
}


void BookStaticsWindow::on_refreshStatsButton_clicked()
{
    loadStatisticsData();
    QMessageBox::information(this, "Statistics Refreshed",
                             "Statistics have been refreshed successfully!");
}

void BookStaticsWindow::on_chartPeriodCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString period = ui->chartPeriodCombo->currentText();
    updateSalesChart(period);
}
*/
