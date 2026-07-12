
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


    qDebug() << "[1] sendRequest called";
    qDebug() << "[1] Command =" << request.getCommandTypeString();
    qDebug() << "[1] Connected =" << m_socket->state();
    if (!isConnected()) {
        qWarning() << "❌ Not connected to server!";
        // emit errorReceived("Not connected to server");
        m_pendingRequests.enqueue(request);
        return;
    }

    // QByteArray data = request.toUtf8();
    QByteArray data = request.toUtf8() + "\n";
    qint64 bytes = m_socket->write(data);
    qDebug() << "[2] Bytes written =" << bytes;
    bool ok = m_socket->flush();
    qDebug() << "[3] Flush =" << ok;


}

void NetworkManager::sendRequest(const QString& command, const QVariantMap& params)
{
    Request request;
    request.setCommandType(Request::stringToCommandType(command));
    request.setParams(params);
    request.setValid(request.getCommandType() != CommandType::Unknown);
    sendRequest(request);
}

void NetworkManager::flushPendingRequests()
{
    if (m_pendingRequests.isEmpty()) {
        return;
    }

    qDebug() << "📤 Flushing" << m_pendingRequests.size() << "queued request(s) after reconnect";

    while (!m_pendingRequests.isEmpty()) {
        Request request = m_pendingRequests.dequeue();
        sendRequest(request);   // socket is connected now, so this sends immediately
    }
}

void NetworkManager::onConnected()
{
    m_isConnected = true;
    qDebug() << "✅ Connected to server:" << m_host << ":" << m_port;
    emit connected();
    flushPendingRequests();
}

void NetworkManager::onDisconnected()
{
    m_isConnected = false;
    qDebug() << "🔌 Disconnected from server";
    emit disconnected();
}


/*
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
*/


void NetworkManager::onReadyRead()
{


    QByteArray newData = m_socket->readAll();
    qDebug() << "[CLIENT IN] Triggered onReadyRead. Bytes read:" << newData.size();
    qDebug() << "[CLIENT IN] Raw data:" << newData;

    m_recvBuffer += newData;

    while (true) {

        int newlineIdx = m_recvBuffer.indexOf('\n');

        if (newlineIdx == -1)
            break;   // پیام کامل نیست، منتظر داده بعدی بمان

        QByteArray messageData = m_recvBuffer.left(newlineIdx);

        m_recvBuffer.remove(0, newlineIdx + 1);


        QJsonDocument doc = QJsonDocument::fromJson(messageData);

        if (!doc.isObject()) {

            qWarning() << "❌ Invalid JSON response!" << messageData;

            emit errorReceived("Invalid JSON response");

            continue;
        }


        Response response = Response::fromJson(doc.object());
        qDebug()<<"we send this to handleresponse";

        handleResponse(response);
    }
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