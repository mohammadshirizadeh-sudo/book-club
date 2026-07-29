#include "publisherwindow.h"
#include "appWindow/ui_publisherwindow.h"

#include "SessionManager.h"
#include "../Server/Request.h"
#include "../Publishers/addbookdialog.h"

#include <QString>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include "../Mutual/notificationwidget.h"

#include <QtCharts/QValueAxis>
#include <QHeaderView>

PublisherWindow::PublisherWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherWindow)
    , m_networkManager(networkManager)
    , m_salesTrendChart(nullptr)
    , m_salesTrendChartView(nullptr)
    , m_ratingsChart(nullptr)
    , m_ratingsChartView(nullptr)
{
    ui->setupUi(this);

    // مقداردهی اولیه نمودارها قبل از setupCharts
    m_salesTrendChart = new QChart();
    m_ratingsChart = new QChart();

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &PublisherWindow::handleResponse);

    setupCharts();
    setupTables();

    // اتصال سیگنال ComboBox به اسلات
    connect(ui->chartPeriodCombo, &QComboBox::currentTextChanged,
            this, &PublisherWindow::on_chartPeriodCombo_currentIndexChanged);
}

PublisherWindow::~PublisherWindow()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &PublisherWindow::handleResponse);
    delete ui;
}

void PublisherWindow::on_pubProfilePushButton_clicked()
{
    emit publisherProfileWindow();
}

void PublisherWindow::on_newBooksPushButton_clicked()
{
    int publisherId = SessionManager::instance()->getUserId();
    AddBookDialog dialog(m_networkManager, publisherId);
    dialog.exec();
}

void PublisherWindow::on_editBooksPushButton_clicked()
{
    emit editWindow();
}

void PublisherWindow::on_discountPushButton_clicked()
{
    emit applydiscountWindow();
}

void PublisherWindow::on_deactivatePushButton_clicked()
{
    emit deactivateBook();
}

void PublisherWindow::on_notifPushButton_clicked()
{
    // ساخت پنجره نوتیفیکیشن به صورت Pop-up روی همین صفحه
    NotificationWidget *notifWidget = new NotificationWidget(m_networkManager, this);

    // تنظیم خصوصیات پنجره (حالت Dialog همراه با دکمه بستن/ضربدر)
    notifWidget->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);

    // حذف خودکار از حافظه پس از بسته شدن پنجره
    notifWidget->setAttribute(Qt::WA_DeleteOnClose);

    // نمایش پنجره
    notifWidget->show();
}
void PublisherWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadAllStatistics();
}

void PublisherWindow::setupCharts()
{
    // اطمینان از اینکه نمودارها مقداردهی شده‌اند
    if (!m_salesTrendChart || !m_ratingsChart) {
        return;
    }

    // تنظیم Sales Trend Chart
    m_salesTrendChartView = new QChartView(m_salesTrendChart, ui->salesChartPlaceholder);
    m_salesTrendChartView->setRenderHint(QPainter::Antialiasing);
    m_salesTrendChartView->setParent(ui->salesChartPlaceholder);

    QVBoxLayout *layoutTrend = new QVBoxLayout(ui->salesChartPlaceholder);
    layoutTrend->setContentsMargins(0, 0, 0, 0);
    layoutTrend->addWidget(m_salesTrendChartView);

    // تنظیم Ratings Chart
    m_ratingsChartView = new QChartView(m_ratingsChart, ui->ratingsChartPlaceholder);
    m_ratingsChartView->setRenderHint(QPainter::Antialiasing);
    m_ratingsChartView->setParent(ui->ratingsChartPlaceholder);

    QVBoxLayout *layoutRatings = new QVBoxLayout(ui->ratingsChartPlaceholder);
    layoutRatings->setContentsMargins(0, 0, 0, 0);
    layoutRatings->addWidget(m_ratingsChartView);

    // تنظیمات پایه نمودارها
    m_salesTrendChart->setTitle("Sales Trend");
    m_salesTrendChart->setAnimationOptions(QChart::SeriesAnimations);
    m_salesTrendChart->legend()->setVisible(true);
    m_salesTrendChart->legend()->setAlignment(Qt::AlignBottom);

    m_ratingsChart->setTitle("Book Ratings");
    m_ratingsChart->setAnimationOptions(QChart::SeriesAnimations);
    m_ratingsChart->legend()->setVisible(false);
}

void PublisherWindow::setupTables()
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

void PublisherWindow::loadAllStatistics()
{
    int publisherId = SessionManager::instance()->getUserId();
    if (publisherId <= 0) return;

    requestSalesOverview();
    requestSalesTrend();
    requestBookRatingsChart();
    requestTopSellingBooks();
    requestBottomSellingBooks();
}

void PublisherWindow::requestSalesOverview()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetSalesOverview, params);
    m_networkManager->sendRequest(request);
}

void PublisherWindow::requestSalesTrend()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["period"] = ui->chartPeriodCombo->currentText(); // Daily, Weekly, Monthly
    params["limit"] = 30;

    Request request(CommandType::GetSalesTrend, params);
    m_networkManager->sendRequest(request);
}

void PublisherWindow::requestBookRatingsChart()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetBookRatingsChart, params);
    m_networkManager->sendRequest(request);
}

void PublisherWindow::requestTopSellingBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["limit"] = 5;

    Request request(CommandType::GetTopSellingBooks, params);
    m_networkManager->sendRequest(request);
}

void PublisherWindow::requestBottomSellingBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["publisherId"] = publisherId;
    params["limit"] = 5;

    Request request(CommandType::GetBottomSellingBooks, params);
    m_networkManager->sendRequest(request);
}

void PublisherWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();


    if (type == CommandType::Logout)
    {
        if (response.getRequestId() != m_pendingLogoutRequestId)
            return;

        m_pendingLogoutRequestId = -1;

        if (!response.isSuccess())
        {
            QMessageBox::warning(
                this,
                "Sign Out",
                "Sign out failed: " + response.getMessage());

            return;
        }

        emit signOutRequested();
        return;
    }

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

void PublisherWindow::updateSalesOverviewUI(const QVariantMap& data)
{
    ui->revenueValueLabel->setText(QString("$%1").arg(data["totalRevenue"].toDouble(), 0, 'f', 2));
    ui->booksValueLabel->setText(QString::number(data["totalBooks"].toInt()));
    ui->ratingValueLabel->setText(QString::number(data["averageRating"].toDouble(), 'f', 1));
    ui->salesValueLabel->setText(QString::number(data["totalSales"].toInt()));
}

void PublisherWindow::updateSalesTrendChartUI(const QVariantMap& data)
{
    if (!m_salesTrendChart) return;

    m_salesTrendChart->removeAllSeries();

    // حذف محورهای موجود
    const auto axes = m_salesTrendChart->axes();
    for (QAbstractAxis *axis : axes) {
        m_salesTrendChart->removeAxis(axis);
    }

    QVariantList labels = data["labels"].toList();
    QVariantList sales = data["sales"].toList();

    if (labels.isEmpty() || sales.isEmpty()) {
        return;
    }

    QLineSeries *series = new QLineSeries();
    series->setName("Units Sold");
    series->setPointsVisible(true);

    QStringList categories;
    int maxSales = 0;

    for (int i = 0; i < labels.size(); ++i) {
        categories.append(labels[i].toString());
        int saleValue = sales[i].toInt();
        series->append(i, saleValue);
        maxSales = qMax(maxSales, saleValue);
    }

    m_salesTrendChart->addSeries(series);

    // تنظیم محور X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    m_salesTrendChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // تنظیم محور Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Sales");
    axisY->setRange(0, qMax(maxSales, 1) + 1);
    m_salesTrendChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_salesTrendChart->legend()->setVisible(true);
    m_salesTrendChart->legend()->setAlignment(Qt::AlignBottom);
}

void PublisherWindow::updateRatingsChartUI(const QVariantMap& data)
{
    if (!m_ratingsChart) return;

    m_ratingsChart->removeAllSeries();

    // حذف محورهای موجود
    const auto axes = m_ratingsChart->axes();
    for (QAbstractAxis *axis : axes) {
        m_ratingsChart->removeAxis(axis);
    }

    QVariantList titles = data["bookTitles"].toList();
    QVariantList ratings = data["ratings"].toList();

    if (titles.isEmpty() || ratings.isEmpty()) {
        return;
    }

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

    // تنظیم محور X
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    m_ratingsChart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // تنظیم محور Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 5);
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Rating");
    m_ratingsChart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_ratingsChart->legend()->setVisible(false);
}

void PublisherWindow::updateTableUI(QTableWidget *table, const QVariantList &books)
{
    if (!table) return;

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

    // تنظیم عرض ستون‌ها
    table->resizeColumnsToContents();
}

void PublisherWindow::on_refreshStatsButton_clicked()
{
    loadAllStatistics();
}

void PublisherWindow::on_chartPeriodCombo_currentIndexChanged(const QString &period)
{
    Q_UNUSED(period);
    requestSalesTrend();
}
void PublisherWindow::on_quitPushButton_clicked()
{
    if (QMessageBox::question(this,
                              "Sign Out",
                              "Sign out of your publisher account?")
        != QMessageBox::Yes)
    {
        return;
    }

    if (!m_networkManager)
    {
        // اگر ارتباط با سرور وجود ندارد،
        // فقط به صورت محلی خارج شو.
        emit openLoginWindow();
        return;
    }

    Request request(CommandType::Logout);

    m_pendingLogoutRequestId = request.getRequestId();

    m_networkManager->sendRequest(request);
}


void PublisherWindow::on_publishedPushButton_clicked()
{
    emit publishedBookWindow();
}

