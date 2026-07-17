
#include "Server.h"
#include "ClientHandler.h"
#include "../Services/AuthService.h"
#include "../Services/UserService.h"
#include "../Services/BookService.h"
#include "../Services/CartService.h"
#include "../Services/PurchaseService.h"
#include "../Services/ReviewService.h"
#include "../Services/PublisherService.h"
#include "../Services/AdminService.h"
#include "../Services/NotificationService.h"
#include "../Repositories/UserRepository.h"
#include "../Repositories/BookRepository.h"
#include "../Repositories/ReviewRepository.h"
#include "../Repositories/PurchaseRepository.h"
#include "../Repositories/LibraryRepository.h"
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer(parent)
    , m_userRepo(nullptr)
    , m_bookRepo(nullptr)
    , m_reviewRepo(nullptr)
    , m_purchaseRepo(nullptr)
    , m_libraryRepo(nullptr)
    , m_authService(nullptr)
    , m_userService(nullptr)
    , m_bookService(nullptr)
    , m_cartService(nullptr)
    , m_purchaseService(nullptr)
    , m_reviewService(nullptr)
    , m_publisherService(nullptr)
    , m_adminService(nullptr)
    , m_notifService(nullptr)
{
    initServices();
}

Server::~Server()
{
    stop();
    cleanupServices();
}

void Server::initServices()
{
    // ===== Repositories =====
    qDebug()<<"mio";
    m_userRepo = new UserRepository(this);
    qDebug()<<"sio";
    m_bookRepo = new BookRepository(this);
    m_reviewRepo = new ReviewRepository(this);
    m_purchaseRepo = new PurchaseRepository(this);
    m_libraryRepo = new LibraryRepository(this);

    // ===== Services =====
    m_notifService = new NotificationService(m_userRepo, this);


    m_authService = new AuthService(m_userRepo, this);
    m_userService = new UserService(m_userRepo, this);
    m_bookService = new BookService(m_bookRepo, m_reviewRepo, this);
    m_cartService = new CartService(m_bookRepo, this);
    m_purchaseService = new PurchaseService(
        m_purchaseRepo, m_bookRepo, m_libraryRepo, m_cartService, m_notifService, this
        );
    m_reviewService = new ReviewService(m_reviewRepo, m_bookRepo, m_notifService, this);
    m_publisherService = new PublisherService(m_bookService,m_bookRepo, m_userRepo, this);
    m_adminService = new AdminService(this);
}

void Server::cleanupServices()
{
    // Qt's parent-child mechanism will delete everything automatically
    // because we passed 'this' as parent to all services
}

bool Server::start(quint16 port)
{
    if (!listen(QHostAddress::LocalHost, port)) {
        qCritical() << "❌ Server could not start on port" << port << ":" << errorString();
        return false;
    }

    qDebug() << "✅ Server started on port" << port;
    return true;
}

void Server::stop()
{
    close();

    for (ClientHandler* client : m_clients.values()) {
        client->deleteLater();
    }
    m_clients.clear();

    qDebug() << "🛑 Server stopped";
}

/*
void Server::incomingConnection(qintptr socketDescriptor)
{

    qDebug() << "[4] incomingConnection";
    qDebug() << "[4] descriptor =" << socketDescriptor;


    QString ipAddress = peerAddress(socketDescriptor).toString();


    emit clientConnected(socketDescriptor, ipAddress);




    ClientHandler* handler = new ClientHandler(
        socketDescriptor,
        m_authService,
        m_bookService,
        m_userService,
        m_purchaseService,
        m_reviewService,
        m_cartService,
        m_publisherService,
        m_adminService,
        this
        );

    connectToClientSignals(handler);

    m_clients[socketDescriptor] = handler;
}

*/


void Server::incomingConnection(qintptr socketDescriptor)
{
    ClientHandler* handler = new ClientHandler(
        socketDescriptor,
        m_authService, m_bookService, m_userService, m_purchaseService,
        m_reviewService, m_cartService, m_publisherService, m_adminService,
        this
        );

    if (!handler || !handler->isValidSocket()) {
        return;
    }

    connectToClientSignals(handler);
    m_clients[socketDescriptor] = handler;

    emit clientConnected(socketDescriptor, handler->peerAddress());
}

void Server::connectToClientSignals(ClientHandler* handler)
{
    if (!handler) return;

    // اتصال سیگنال‌های ClientHandler به سیگنال‌های Server
    connect(handler, &ClientHandler::requestReceived,
            this, [this](const QString& request) {
                emit requestReceived(request);

            });

    connect(handler, &ClientHandler::responseSent,
            this, [this](const QString& response) {
                emit responseSent(response);
            });

    connect(handler, &ClientHandler::clientError,
            this, [this](const QString& error) {
                emit errorOccurred(error);
            });

    connect(handler, &ClientHandler::disconnected,
            this, [this, handler]() {
                emit clientDisconnected(handler->m_socketDescriptor);
                emit systemEvent("Client disconnected");
            });
}

bool Server::startServer(quint16 port)
{
    return start(port);  // ← همان start موجود را صدا می‌زند
}


void Server::stopServer()
{
    stop();  // ← همان stop موجود را صدا می‌زند
}


