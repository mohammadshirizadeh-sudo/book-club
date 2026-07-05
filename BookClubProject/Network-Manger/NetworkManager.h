// Client/NetworkManager.h
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QTcpSocket>
#include <QObject>
#include "../Server/Request.h"
#include "../Server/Response.h"


class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();


    bool connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;
    QString getHost() const { return m_host; }
    quint16 getPort() const { return m_port; }

    // ===== Send Request =====
    void sendRequest(const Request& request);
    void sendRequest(const QString& command, const QVariantMap& params = QVariantMap());

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);


    void responseReceived(const Response& response);
    void successReceived(const QVariantMap& data);
    void errorReceived(const QString& message);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* m_socket;
    QString m_host;
    quint16 m_port;
    bool m_isConnected;

    void handleResponse(const Response& response);
    void emitSignals(const Response& response);
};

#endif