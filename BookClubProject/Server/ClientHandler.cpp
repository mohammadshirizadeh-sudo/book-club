
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
    m_socket = new QTcpSocket(this);

    if (!m_socket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor:" << socketDescriptor;
        m_socket->close();
        m_socket->deleteLater();
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

    m_socket->close();
    emit disconnected();
    deleteLater();
}

void ClientHandler::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString requestData = QString::fromUtf8(data).trimmed();

    if (!requestData.isEmpty()) {

        QFuture<void> future = QtConcurrent::run([this, requestData]() {
            processRequest(requestData);
        });
    }
}

void ClientHandler::onDisconnected()
{
    qDebug() << "Client disconnected:" << m_socketDescriptor;
    emit disconnected();
    deleteLater();
}

// ===== Core Methods =====

// ClientHandler.cpp
void ClientHandler::handleRequest(const QString& requestData)
{
    // 1. Parse request
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponse(Response::error("Invalid request format"));
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
        m_adminService,this
        ));

    if (!command) {
        sendResponse(Response::error("Unknown command: " + request.getCommandTypeString()));
        return;
    }


    try {
        Response response = command->execute(request.getParams());
        sendResponse(response);
    }
    catch (const std::exception& e) {

        qCritical() << "❌ Command execution failed:" << e.what();
        sendResponse(Response::error("Internal error: " + QString(e.what())));
    }
    catch (...) {

        qCritical() << "❌ Unknown command execution error!";
        sendResponse(Response::error("Internal server error"));
    }
}


void ClientHandler::handleRequestSync(const QString& requestData)
{
    // 1. Parse request
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponseSync(Response::error("Invalid request format"));
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
        m_adminService,this
        ));
    if (!command) {
        sendResponseSync(Response::error("Unknown command"));
        return;
    }

    try {
        Response response = command->execute(request.getParams());
        sendResponseSync(response);
    } catch (const std::exception& e) {
        sendResponseSync(Response::error("Internal error: " + QString(e.what())));
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
        QByteArray data = response.toUtf8();
        m_socket->write(data);
        m_socket->flush();
        qDebug() << "Response sent to client:" << response.left(100) << "...";
    }
}


void ClientHandler::sendResponse(const Response& response)
{
    sendResponse(response.toJsonString());
}


void ClientHandler::setSession(int userId, UserRole role)
{
    m_sessionUserId = userId;
    m_sessionRole = role;
    m_isAuthenticated = true;
}



void ClientHandler::processRequest(const QString& requestData)
{
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        emit responseReady(Response::error("Invalid request format"));
        return;
    }
    std::unique_ptr<Command> command(CommandFactory::create(
        request.getCommandType(),
        m_authService,
        m_bookService,
        m_userService,
        m_purchaseService,
        m_reviewService,
        m_cartService,
        m_publisherService,
        m_adminService,this
        ));
    if (!command) {
        emit responseReady(Response::error("Unknown command"));
        return;
    }


    try {
        Response response = command->execute(request.getParams());
        emit responseReady(response);
    } catch (const std::exception& e) {
        emit responseReady(Response::error("Internal error: " + QString(e.what())));
    } catch (...) {
        emit responseReady(Response::error("Internal server error"));
    }
}


void ClientHandler::onResponseReady(const Response& response)
{
    sendResponse(response);
}



