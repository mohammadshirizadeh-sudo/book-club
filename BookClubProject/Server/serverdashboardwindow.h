#ifndef SERVERDASHBOARDWINDOW_H
#define SERVERDASHBOARDWINDOW_H

#include <QWidget>
#include <QTimer>
#include "../Server/server.h"

namespace Ui {
class ServerDashboardWindow;
}

class ServerDashboardWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ServerDashboardWindow(QWidget *parent = nullptr);
    ~ServerDashboardWindow();

    bool isServerRunning() const;

signals:
    void backRequested();

private slots:
    void on_backButton_clicked();
    void on_startServerButton_clicked();
    void on_stopServerButton_clicked();
    void on_refreshStatsButton_clicked();
    void on_clearLogsButton_clicked();

    // Timer slots
    void updateCurrentTime();
    void updateUptime();
    void autoRefreshStats();

    // Server signal handlers
    void onClientConnected(const QString &socketId, const QString &ipAddress);
    void onClientDisconnected(const QString &socketId);
    void onRequestReceived(const QString &requestInfo);
    void onResponseSent(const QString &responseInfo);
    void onServerError(const QString &errorMessage);
    void onSystemEvent(const QString &eventMessage);

private:
    Ui::ServerDashboardWindow *ui;
    Server *m_server;
    QTimer *m_timeTimer;
    QTimer *m_uptimeTimer;
    QTimer *m_statsTimer;
    QDateTime m_startTime;

    bool startServer(int port = 1234);
    bool stopServer();
    void setupClientsTable();
    void updateServerStatus(bool running);
    void logRequest(const QString &message, const QString &type = "INFO");
    void logEvent(const QString &message, const QString &level = "INFO");
    void refreshStatistics();
};

#endif // SERVERDASHBOARDWINDOW_H
