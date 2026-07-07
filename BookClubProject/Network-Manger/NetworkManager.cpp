
#include "NetworkManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_host("")
    , m_port(0)
    , m_isConnected(false)
{

    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onErrorOccurred);
}

NetworkManager::~NetworkManager()
{
    disconnectFromServer();
}


bool NetworkManager::connectToServer(const QString& host, quint16 port)
{
    if (m_isConnected) {
        qWarning() << "Already connected to server";
        return true;
    }

    m_host = host;
    m_port = port;

    qDebug() << "🔌 Connecting to server:" << host << ":" << port;
    m_socket->connectToHost(host, port);
    return true;
}

void NetworkManager::disconnectFromServer()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        if (m_socket->state() == QTcpSocket::ConnectedState) {
            m_socket->waitForDisconnected(1000);
        }
        m_socket->close();
    }
    m_isConnected = false;
    qDebug() << "🔌 Disconnected from server";
}

bool NetworkManager::isConnected() const
{
    return m_isConnected && m_socket->state() == QTcpSocket::ConnectedState;
}


void NetworkManager::sendRequest(const Request& request)
{
    if (!isConnected()) {
        qWarning() << "❌ Not connected to server!";
        emit errorReceived("Not connected to server");
        return;
    }

    QByteArray data = request.toUtf8();
    m_socket->write(data);
    m_socket->flush();

    qDebug() << "📤 Request sent:" << request.getCommandTypeString();
}

void NetworkManager::sendRequest(const QString& command, const QVariantMap& params)
{
    Request request;
    request.setCommandType(Request::stringToCommandType(command));
    request.setParams(params);
    request.setValid(true);
    sendRequest(request);
}

void NetworkManager::onConnected()
{
    m_isConnected = true;
    qDebug() << "✅ Connected to server:" << m_host << ":" << m_port;
    emit connected();
}

void NetworkManager::onDisconnected()
{
    m_isConnected = false;
    qDebug() << "🔌 Disconnected from server";
    emit disconnected();
}

void NetworkManager::onReadyRead()
{
    QByteArray data = m_socket->readAll();

    qDebug() << "📥 Raw data received:" << data.left(100) << "...";

    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "❌ Invalid JSON response!";
        emit errorReceived("Invalid JSON response");
        return;
    }

    // Convert to Response
    Response response = Response::fromJson(doc.object());
    handleResponse(response);
}

void NetworkManager::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    QString errorMsg = m_socket->errorString();
    qCritical() << "❌ Socket error:" << errorMsg;
    m_isConnected = false;
    emit connectionError(errorMsg);
    emit errorReceived(errorMsg);
}

void NetworkManager::handleResponse(const Response& response)
{
    qDebug() << "📥 Response received - Success:" << response.isSuccess();

    emit responseReceived(response);
    emitSignals(response);
}

void NetworkManager::emitSignals(const Response& response)
{
    if (response.isSuccess()) {
        emit successReceived(response.getData());
    } else {
        emit errorReceived(response.getMessage());
    }
}