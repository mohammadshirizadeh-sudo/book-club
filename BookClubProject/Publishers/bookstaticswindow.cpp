#include "bookstaticswindow.h"
#include "Publishers/ui_bookstaticswindow.h"

#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include "../Network-Manger/NetworkManager.h"

BookStaticsWindow::BookStaticsWindow(NetworkManager* networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BookStaticsWindow),
    m_networkManager(networkManager),
    m_salesTrendChart(new QChart()),
    m_ratingsChart(new QChart())
{
    ui->setupUi(this);
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &BookStaticsWindow::handleResponse);
    setupCharts();
    setupTables();
}

BookStaticsWindow::~BookStaticsWindow()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &BookStaticsWindow::handleResponse);
    delete ui;
}

void BookStaticsWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadAllStatistics();
}

void BookStaticsWindow::setupCharts()
{
    m_salesTrendChartView = new QChartView(m_salesTrendChart, ui->salesChartPlaceholder);
    m_salesTrendChartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layoutTrend = new QVBoxLayout(ui->salesChartPlaceholder);
    layoutTrend->setContentsMargins(0, 0, 0, 0);
    layoutTrend->addWidget(m_salesTrendChartView);
    m_ratingsChartView = new QChartView(m_ratingsChart, ui->ratingsChartPlaceholder);
    m_ratingsChartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layoutRatings = new QVBoxLayout(ui->ratingsChartPlaceholder);
    layoutRatings->setContentsMargins(0, 0, 0, 0);
    layoutRatings->addWidget(m_ratingsChartView);
}
void BookStaticsWindow::setupTables()
{
    auto configureTable = [](QTableWidget *table) {
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"Rank", "Book Title", "Sales", "Revenue"});
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
    };

    configureTable(ui->bestSellersTable);
    configureTable(ui->worstSellersTable);
}
void BookStaticsWindow::loadAllStatistics()
{
    int publisherId = SessionManager::instance()->getUserId();
    if (publisherId <= 0) return;

    requestSalesOverview();
    requestSalesTrend();
    requestBookRatingsChart();
    requestTopSellingBooks();
    requestBottomSellingBooks();
}
void BookStaticsWindow::requestSalesOverview()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetSalesOverview, params);
    m_networkManager->sendRequest(request);
}
void BookStaticsWindow::requestSalesTrend()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["period"] = ui->chartPeriodCombo->currentText(); // Daily, Weekly, Monthly
    params["limit"] = 30;

    Request request(CommandType::GetSalesTrend, params);
    m_networkManager->sendRequest(request);
}
void BookStaticsWindow::requestBookRatingsChart()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetBookRatingsChart, params);
    m_networkManager->sendRequest(request);
}
void BookStaticsWindow::requestTopSellingBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["limit"] = 5;

    Request request(CommandType::GetTopSellingBooks, params);
    m_networkManager->sendRequest(request);
}
void BookStaticsWindow::requestBottomSellingBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["limit"] = 5;

    Request request(CommandType::GetBottomSellingBooks, params);
    m_networkManager->sendRequest(request);
}
void BookStaticsWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // فیلتر کردن دستورات مرتبط با این صفحه
    if (type != CommandType::GetSalesOverview &&
        type != CommandType::GetSalesTrend &&
        type != CommandType::GetBookRatingsChart &&
        type != CommandType::GetTopSellingBooks &&
        type != CommandType::GetBottomSellingBooks) {
        return;
    }

    if (!response.isSuccess()) {
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    if (type == CommandType::GetSalesOverview) {
        updateSalesOverviewUI(data);
    }
    else if (type == CommandType::GetSalesTrend) {
        updateSalesTrendChartUI(data);
    }
    else if (type == CommandType::GetBookRatingsChart) {
        updateRatingsChartUI(data);
    }
    else if (type == CommandType::GetTopSellingBooks) {
        updateTableUI(ui->bestSellersTable, data["books"].toList());
    }
    else if (type == CommandType::GetBottomSellingBooks) {
        updateTableUI(ui->worstSellersTable, data["books"].toList());
    }
}

void BookStaticsWindow::updateSalesOverviewUI(const QVariantMap& data)
{
    ui->revenueValueLabel->setText(QString("$%1").arg(data["totalRevenue"].toDouble(), 0, 'f', 2));
    ui->booksValueLabel->setText(QString::number(data["totalBooks"].toInt()));
    ui->ratingValueLabel->setText(QString::number(data["averageRating"].toDouble(), 'f', 1));
    ui->salesValueLabel->setText(QString::number(data["totalSales"].toInt()));
}

void BookStaticsWindow::updateSalesTrendChartUI(const QVariantMap& data)
{
    m_salesTrendChart->removeAllSeries();
    for (auto *axis : m_salesTrendChart->axes()) {
        m_salesTrendChart->removeAxis(axis);
    }

    QVariantList labels = data["labels"].toList();
    QVariantList sales = data["sales"].toList();

    QLineSeries *series = new QLineSeries();
    series->setName("Units Sold");
    series->setPointsVisible(true);

    QStringList categories;
    for (int i = 0; i < labels.size(); ++i) {
        categories.append(labels[i].toString());
        series->append(i, sales[i].toInt());
    }

    m_salesTrendChart->addSeries(series);

    if (!categories.isEmpty()) {
        int maxSales = 0;

        for (int i = 0; i < sales.size(); ++i)
        {
            maxSales = qMax(maxSales, sales[i].toInt());
        }
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        m_salesTrendChart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setLabelFormat("%d");
        axisY->setTitleText("Sales");
        axisY->setRange(0, qMax(maxSales, 1) + 1);
        m_salesTrendChart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
    }

    m_salesTrendChart->legend()->setVisible(true);
    m_salesTrendChart->legend()->setAlignment(Qt::AlignBottom);
}
void BookStaticsWindow::updateRatingsChartUI(const QVariantMap& data)
{
    m_ratingsChart->removeAllSeries();
    for (auto *axis : m_ratingsChart->axes()) {
        m_ratingsChart->removeAxis(axis);
    }

    QVariantList titles = data["bookTitles"].toList();
    QVariantList ratings = data["ratings"].toList();

    QBarSet *set = new QBarSet("Rating");
    QStringList categories;

    for (int i = 0; i < titles.size(); ++i) {
        *set << ratings[i].toDouble();

        QString title = titles[i].toString();
        if (title.length() > 10) {
            title = title.left(8) + "..";
        }
        categories.append(title);
    }

    QBarSeries *series = new QBarSeries();
    series->append(set);
    m_ratingsChart->addSeries(series);

    if (!categories.isEmpty()) {
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        m_ratingsChart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, 5);
        axisY->setLabelFormat("%.1f");
        m_ratingsChart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
    }

    m_ratingsChart->legend()->setVisible(false);
}
void BookStaticsWindow::updateTableUI(QTableWidget *table, const QVariantList &books)
{
    table->setRowCount(0);

    for (int i = 0; i < books.size(); ++i) {
        QVariantMap book = books[i].toMap();
        table->insertRow(i);

        QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(book["rank"].toInt()));
        QTableWidgetItem *titleItem = new QTableWidgetItem(book["title"].toString());
        QTableWidgetItem *salesItem = new QTableWidgetItem(QString::number(book["salesCount"].toInt()));
        QTableWidgetItem *revenueItem = new QTableWidgetItem(QString("$%1").arg(book["revenue"].toDouble(), 0, 'f', 2));

        rankItem->setTextAlignment(Qt::AlignCenter);
        titleItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        salesItem->setTextAlignment(Qt::AlignCenter);
        revenueItem->setTextAlignment(Qt::AlignCenter);

        table->setItem(i, 0, rankItem);
        table->setItem(i, 1, titleItem);
        table->setItem(i, 2, salesItem);
        table->setItem(i, 3, revenueItem);
    }
}
void BookStaticsWindow::on_refreshStatsButton_clicked()
{
    loadAllStatistics();
}
void BookStaticsWindow::on_chartPeriodCombo_currentIndexChanged(const QString &period)
{
    Q_UNUSED(period);
    requestSalesTrend();
}
void BookStaticsWindow::on_quitPushButton_clicked()
{
    this->close();
}