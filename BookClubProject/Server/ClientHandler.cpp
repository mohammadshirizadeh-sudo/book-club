
// ClientHandler.cpp

#include <QDebug>
#include "ClientHandler.h"
#include <QtConcurrent>






// ===== Constructor =====
ClientHandler::ClientHandler(qintptr socketDescriptor,
                             AuthService* authService,
                             BookService* bookService,
                             UserService* userService,
                             PurchaseService* purchaseService,
                             ReviewService* reviewService,
                             CartService* cartService,
                             PublisherService* publisherService,
                             AdminService* adminService,
                             QObject *parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_authService(authService)
    , m_bookService(bookService)
    , m_userService(userService)
    , m_purchaseService(purchaseService)
    , m_reviewService(reviewService)
    , m_cartService(cartService)
    , m_publisherService(publisherService)
    , m_adminService(adminService)
{

    qDebug() << "[5] ClientHandler created";
    m_socket = new QTcpSocket(this);

    if (!m_socket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor:" << socketDescriptor;
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
        deleteLater();
        return;
    }

    m_parser = new RequestParser(this);

    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &ClientHandler::onSocketError);
    connect(this, &ClientHandler::responseReady, this, &ClientHandler::onResponseReady);


    qDebug() << "ClientHandler created for socket:" << socketDescriptor;
}

ClientHandler::~ClientHandler()
{
    if (m_socket) {
        m_socket->close();
    }
    qDebug() << "ClientHandler destroyed for socket:" << m_socketDescriptor;
}

// ===== Slots =====


void ClientHandler::onSocketError(QAbstractSocket::SocketError socketError)
{
    qWarning() << "⚠️ Socket error:" << m_socket->errorString()
        << "(Code:" << socketError << ")";

    emit clientError(m_socket->errorString());

    m_socket->close();
    onDisconnected();
}

/*
void ClientHandler::onReadyRead() {
    QByteArray data = m_socket->readAll();
    qDebug() << "📥 Server received:" << data;
    QString requestData = QString::fromUtf8(data).trimmed();
    if (requestData.isEmpty()) return;

    m_pendingTasks.fetchAndAddOrdered(1);

    QFuture<void> future  = QtConcurrent::run([this, requestData]() {
        if (!m_isDestroying.loadAcquire()) {
            processRequest(requestData);
        }
        m_pendingTasks.fetchAndSubOrdered(1);
    });
}
*/



void ClientHandler::onReadyRead()
{


    QByteArray newData = m_socket->readAll();
    qDebug() << "[SERVER IN] Triggered onReadyRead. Bytes read:" << newData.size();
    qDebug() << "[SERVER IN] Raw data:" << newData.left(100)<<"...";

    m_recvBuffer += newData;

    while (true) {

        int idx = m_recvBuffer.indexOf('\n');

        if (idx == -1)
            break;


        QByteArray messageData =
            m_recvBuffer.left(idx);

        m_recvBuffer.remove(0, idx + 1);


        QString requestData =
            QString::fromUtf8(messageData).trimmed();


        if (requestData.isEmpty())
            continue;


        m_pendingTasks.fetchAndAddOrdered(1);


        QtConcurrent::run([this, requestData]() {

            if (!m_isDestroying.loadAcquire()) {
                processRequest(requestData);
            }

            m_pendingTasks.fetchAndSubOrdered(1);
        });
    }
}

void ClientHandler::onDisconnected() {
    m_isDestroying.storeRelease(1);
    emit disconnected();

    if (m_pendingTasks.loadAcquire() > 0) {
        QTimer::singleShot(100, this, &ClientHandler::onDisconnected);
        return;
    }

    deleteLater();
}

void ClientHandler::handleRequest(const QString& requestData)
{
    // 1. Parse request
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponse( Response::error(request.getCommandType(),"Invalid request format"));
        return;
    }

    // 2. Create command using CommandFactory
    std::unique_ptr<Command> command(CommandFactory::create(
        request.getCommandType(),
        m_authService,
        m_bookService,
        m_userService,
        m_purchaseService,
        m_reviewService,
        m_cartService,
        m_publisherService,
        m_adminService,m_notificationService,m_libraryService,this

        ));

    if (!command) {
        sendResponse(Response::error(request.getCommandType() , "Unknown command: " + request.getCommandTypeString()));
        return;
    }


    try {
        Response response = command->execute(request.getParams());
        sendResponse(response);
    }
    catch (const std::exception& e) {

        qCritical() << "❌ Command execution failed:" << e.what();
        sendResponse(Response::error(request.getCommandType(), "Internal error: " + QString(e.what())));
    }
    catch (...) {

        qCritical() << "❌ Unknown command execution error!";
        sendResponse(Response::error(request.getCommandType(),"Internal server error"));
    }
}


void ClientHandler::handleRequestSync(const QString& requestData)
{
    // 1. Parse request
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponseSync(Response::error(request.getCommandType(),"Invalid request format"));
        return;
    }

    // 2. Create command
    std::unique_ptr<Command> command(CommandFactory::create(
        request.getCommandType(),
        m_authService,
        m_bookService,
        m_userService,
        m_purchaseService,
        m_reviewService,
        m_cartService,
        m_publisherService,
        m_adminService,m_notificationService ,m_libraryService,  this
        ));
    if (!command) {
        sendResponseSync(Response::error(request.getCommandType(),"Unknown command"));
        return;
    }

    try {
        Response response = command->execute(request.getParams());
        sendResponseSync(response);
    } catch (const std::exception& e) {
        sendResponseSync(Response::error(request.getCommandType(),"Internal error: " + QString(e.what())));
    }
}

void ClientHandler::sendResponseSync(const Response& response)
{

    QMetaObject::invokeMethod(this, "sendResponse",
                              Qt::QueuedConnection,
                              Q_ARG(Response, response));
}


void ClientHandler::sendResponse(const QString& response)
{
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        QByteArray data = response.toUtf8() + "\n";
        qint64 bytesWritten = m_socket->write(data);
        m_socket->flush();

        qDebug() << "[SERVER OUT] Bytes written:" << bytesWritten;
        qDebug() << "[SERVER OUT] Raw payload:" << data.left(100)<<"...";
    } else {
        qWarning() << "[SERVER OUT ERROR] Socket is not connected! State:" << (m_socket ? m_socket->state() : -1);
    }
}


void ClientHandler::sendResponse(const Response& response)
{
    sendResponse(response.toJsonString());
    emit responseSent(response.toJsonString());

}


void ClientHandler::setSession(int userId, UserRole role)
{
    QMutexLocker locker(&m_sessionMutex);
    m_sessionUserId = userId;
    m_sessionRole = role;
    m_isAuthenticated = true;
}



void ClientHandler::processRequest(const QString& requestData)
{


    qDebug() << "[SERVER PROCESS] Attempting to parse extracted string:" << requestData.left(100)<<"...";
    Request request = m_parser->parse(requestData);
    qDebug() << "[SERVER PROCESS] Parsed successfully? Valid:" << request.isValid() << "Command:" << request.getCommandTypeString();
    if (!request.isValid()) {
        emit responseReady(Response::error(request.getCommandType(),"Invalid request format"));
        return;
    }

    emit requestReceived(requestData);


    std::unique_ptr<Command> command(CommandFactory::create(
        request.getCommandType(),
        m_authService,
        m_bookService,
        m_userService,
        m_purchaseService,
        m_reviewService,
        m_cartService,
        m_publisherService,
        m_adminService,m_notificationService,m_libraryService, this

        ));
    if (!command) {
        emit responseReady(Response::error(request.getCommandType(),"Unknown command"));
        return;
    }


    try {
        Response response = command->execute(request.getParams());
        emit responseReady(response);
    } catch (const std::exception& e) {
        emit responseReady(Response::error(request.getCommandType(),"Internal error: " + QString(e.what())));
    } catch (...) {
        emit responseReady(Response::error(request.getCommandType(),"Internal server error"));
    }
}


void ClientHandler::onResponseReady(const Response& response)
{
    sendResponse(response);
}


void ClientHandler::disconnectFromClient()
{
    if (m_isDestroying.loadAcquire()) {
        return;
    }

    m_isDestroying.storeRelease(1);

    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->close();
    }

    emit disconnected();

    if (m_pendingTasks.loadAcquire() > 0) {
        QTimer::singleShot(50, this, &ClientHandler::disconnectFromClient);
        return;
    }

    deleteLater();
}


