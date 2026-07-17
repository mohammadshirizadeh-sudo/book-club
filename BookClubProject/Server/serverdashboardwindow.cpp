#include "serverdashboardwindow.h"
#include "Server/ui_serverdashboardwindow.h"

#include "../Server/server.h"

#include <QDateTime>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>

ServerDashboardWindow::ServerDashboardWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerDashboardWindow)
    , m_server(nullptr)
    , m_timeTimer(new QTimer(this))
    , m_uptimeTimer(new QTimer(this))
    , m_statsTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Setup clients table
    setupClientsTable();

    // Setup timers
    connect(m_timeTimer, &QTimer::timeout, this, &ServerDashboardWindow::updateCurrentTime);
    connect(m_uptimeTimer, &QTimer::timeout, this, &ServerDashboardWindow::updateUptime);
    connect(m_statsTimer, &QTimer::timeout, this, &ServerDashboardWindow::autoRefreshStats);

    // Start time timer (every second)
    m_timeTimer->start(1000);
    updateCurrentTime();

    // Update uptime every second when server is running
    m_uptimeTimer->start(1000);

    // Auto-refresh stats every 5 seconds
    m_statsTimer->start(5000);

    // Initial stats update
    refreshStatistics();
}

ServerDashboardWindow::~ServerDashboardWindow()
{
    if (m_server && m_server->isRunning()) {
        stopServer();
    }
    delete ui;
}

bool ServerDashboardWindow::isServerRunning() const
{
    return m_server && m_server->isRunning();
}

void ServerDashboardWindow::setupClientsTable()
{
    ui->clientsTableWidget->setColumnCount(3);
    QStringList headers;
    headers << "Socket ID" << "IP Address" << "Username";
    ui->clientsTableWidget->setHorizontalHeaderLabels(headers);
    ui->clientsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->clientsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->clientsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->clientsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ServerDashboardWindow::updateCurrentTime()
{
    ui->currentTimeLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
}

void ServerDashboardWindow::updateUptime()
{
    if (m_server && m_server->isRunning()) {
        qint64 seconds = m_startTime.secsTo(QDateTime::currentDateTime());
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        int secs = seconds % 60;
        ui->uptimeLabel->setText(QString("%1:%2:%3")
                                     .arg(hours, 2, 10, QChar('0'))
                                     .arg(minutes, 2, 10, QChar('0'))
                                     .arg(secs, 2, 10, QChar('0')));
    }
}

void ServerDashboardWindow::autoRefreshStats()
{
    if (m_server && m_server->isRunning()) {
        refreshStatistics();
    }
}

void ServerDashboardWindow::refreshStatistics()
{
    // Update online users count
    ui->onlineUsersLabel->setText(QString::number(ui->clientsTableWidget->rowCount()));

    // Simulated CPU/RAM values (in real app, would get from system)
    static float cpuUsage = 0.0f;
    static float ramUsage = 30.0f;

    cpuUsage = qMin(100.0f, cpuUsage + (qrand() % 20 - 10) * 0.1f);
    ramUsage = qMin(95.0f, qMax(25.0f, ramUsage + (qrand() % 6 - 3) * 0.1f));

    ui->cpuValueLabel->setText(QString("%1%").arg(cpuUsage, 0, 'f', 1));
    ui->cpuProgressBar->setValue(static_cast<int>(cpuUsage));

    ui->ramValueLabel->setText(QString("%1%").arg(ramUsage, 0, 'f', 1));
    ui->ramProgressBar->setValue(static_cast<int>(ramUsage));
}

void ServerDashboardWindow::updateServerStatus(bool running)
{
    if (running) {
        ui->statusValueLabel->setText("RUNNING");
        ui->statusValueLabel->setStyleSheet(
            "QLabel {"
            "background-color: rgb(100, 255, 100);"
            "border: 3px solid black;"
            "border-radius: 12px;"
            "font: 700 12pt \"Script MT Bold\";"
            "font-size: 24px;"
            "color: white;"
            "padding: 5px;"
            "}"
            );

        ui->startServerButton->setEnabled(false);
        ui->stopServerButton->setEnabled(true);
        ui->portSpinBox->setEnabled(false);

        logEvent("Server started successfully", "SUCCESS");
    } else {
        ui->statusValueLabel->setText("STOPPED");
        ui->statusValueLabel->setStyleSheet(
            "QLabel {"
            "background-color: rgb(255, 100, 100);"
            "border: 3px solid black;"
            "border-radius: 12px;"
            "font: 700 12pt \"Script MT Bold\";"
            "font-size: 24px;"
            "color: white;"
            "padding: 5px;"
            "}"
            );

        ui->startServerButton->setEnabled(true);
        ui->stopServerButton->setEnabled(false);
        ui->portSpinBox->setEnabled(true);

        ui->uptimeLabel->setText("--:--:--");
        logEvent("Server stopped", "WARNING");
    }
}

bool ServerDashboardWindow::startServer(int port)
{
    if (!m_server) {
        m_server = new Server(this);

        // Connect server signals
        connect(m_server, &Server::clientConnected,
                this, &ServerDashboardWindow::onClientConnected);
        connect(m_server, &Server::clientDisconnected,
                this, &ServerDashboardWindow::onClientDisconnected);
        connect(m_server, &Server::requestReceived,
                this, &ServerDashboardWindow::onRequestReceived);
        connect(m_server, &Server::responseSent,
                this, &ServerDashboardWindow::onResponseSent);
        connect(m_server, &Server::errorOccurred,
                this, &ServerDashboardWindow::onServerError);
        connect(m_server, &Server::systemEvent,
                this, &ServerDashboardWindow::onSystemEvent);
    }

    bool success = m_server->startServer(port);

    if (success) {
        m_startTime = QDateTime::currentDateTime();
        updateServerStatus(true);
        logRequest(QString("Server started on port %1").arg(port), "SYSTEM");
    } else {
        QMessageBox::critical(this, "Error",
                              QString("Failed to start server on port %1").arg(port));
        logEvent("Failed to start server", "ERROR");
    }

    return success;
}

bool ServerDashboardWindow::stopServer()
{
    if (m_server && m_server->isRunning()) {
        m_server->stopServer();
        updateServerStatus(false);
        logRequest("Server stopped by user", "SYSTEM");
        return true;
    }
    return false;
}

void ServerDashboardWindow::logRequest(const QString &message, const QString &type)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString color;

    if (type == "REQUEST") color = "color: blue;";
    else if (type == "RESPONSE") color = "color: green;";
    else if (type == "ERROR") color = "color: red;";
    else color = "color: black;";

    ui->requestLogTextEdit->append(
        QString("<span style='color:gray;'>[%1]</span> "
                "<span style='%2'>[%3]</span> %4")
            .arg(timestamp).arg(color).arg(type).arg(message)
        );

    // Auto-scroll to bottom
    QTextCursor cursor = ui->requestLogTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->requestLogTextEdit->setTextCursor(cursor);
}

void ServerDashboardWindow::logEvent(const QString &message, const QString &level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color;

    if (level == "SUCCESS") color = "color: green; font-weight: bold;";
    else if (level == "WARNING") color = "color: orange; font-weight: bold;";
    else if (level == "ERROR") color = "color: red; font-weight: bold;";
    else color = "color: black;";

    ui->eventLogTextEdit->append(
        QString("<span style='color:gray;'>[%1]</span> "
                "<span style='%2'>[%3]</span> %4")
            .arg(timestamp).arg(color).arg(level).arg(message)
        );

    // Auto-scroll to bottom
    QTextCursor cursor = ui->eventLogTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->eventLogTextEdit->setTextCursor(cursor);
}

// ===== Slot Implementations =====

void ServerDashboardWindow::on_backButton_clicked()
{
    emit backRequested();
}

void ServerDashboardWindow::on_startServerButton_clicked()
{
    int port = ui->portSpinBox->value();
    startServer(port);
}

void ServerDashboardWindow::on_stopServerButton_clicked()
{
    stopServer();
}

void ServerDashboardWindow::on_refreshStatsButton_clicked()
{
    refreshStatistics();
    logEvent("Statistics refreshed manually", "INFO");
}

void ServerDashboardWindow::on_clearLogsButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Clear",
        "Are you sure you want to clear all logs?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        ui->requestLogTextEdit->clear();
        ui->eventLogTextEdit->clear();
        logEvent("Logs cleared by user", "INFO");
    }
}

// ===== Server Event Handlers =====

void ServerDashboardWindow::onClientConnected(const QString &socketId, const QString &ipAddress)
{
    int row = ui->clientsTableWidget->rowCount();
    ui->clientsTableWidget->insertRow(row);

    ui->clientsTableWidget->setItem(row, 0, new QTableWidgetItem(socketId));
    ui->clientsTableWidget->setItem(row, 1, new QTableWidgetItem(ipAddress));
    ui->clientsTableWidget->setItem(row, 2, new QTableWidgetItem("--"));

    ui->onlineUsersLabel->setText(QString::number(ui->clientsTableWidget->rowCount()));

    logEvent(QString("Client connected: %1 (%2)").arg(ipAddress).arg(socketId), "SUCCESS");
}

void ServerDashboardWindow::onClientDisconnected(const QString &socketId)
{
    for (int i = 0; i < ui->clientsTableWidget->rowCount(); ++i) {
        QTableWidgetItem *idItem = ui->clientsTableWidget->item(i, 0);
        if (idItem && idItem->text() == socketId) {
            ui->clientsTableWidget->removeRow(i);
            break;
        }
    }

    ui->onlineUsersLabel->setText(QString::number(ui->clientsTableWidget->rowCount()));

    logEvent(QString("Client disconnected: %1").arg(socketId), "WARNING");
}

void ServerDashboardWindow::onRequestReceived(const QString &requestInfo)
{
    logRequest(requestInfo, "REQUEST");

    // Update request count
    int currentRequests = ui->totalRequestsLabel->text().toInt();
    ui->totalRequestsLabel->setText(QString::number(currentRequests + 1));
}

void ServerDashboardWindow::onResponseSent(const QString &responseInfo)
{
    logRequest(responseInfo, "RESPONSE");

    // Update response count
    int currentResponses = ui->totalResponsesLabel->text().toInt();
    ui->totalResponsesLabel->setText(QString::number(currentResponses + 1));
}

void ServerDashboardWindow::onServerError(const QString &errorMessage)
{
    logEvent(errorMessage, "ERROR");
    QMessageBox::warning(this, "Server Error", errorMessage);
}

void ServerDashboardWindow::onSystemEvent(const QString &eventMessage)
{
    logEvent(eventMessage, "INFO");
}
