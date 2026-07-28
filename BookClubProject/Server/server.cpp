
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
    delete m_resourceMonitor;
}

void Server::initServices()
{

    m_userRepo = new UserRepository(this);
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
        m_purchaseRepo, m_bookRepo, m_libraryRepo, m_cartService, m_notifService,m_bookService, this
        );
    m_reviewService = new ReviewService(m_reviewRepo, m_bookRepo, m_notifService, this);
    m_publisherService = new PublisherService(m_bookService,m_bookRepo, m_userRepo, this);
    m_libraryService = new LibraryService(m_libraryRepo , this);
    m_adminService = new AdminService( m_userService, m_bookService, m_reviewService, m_purchaseService, m_notifService, m_libraryService, this);
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


    m_startTime = QDateTime::currentDateTime();

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
        m_libraryService,m_notifService,
        this
        );

    if (!handler || !handler->isValidSocket()) {
        return;
    }

    connectToClientSignals(handler);
    m_clients[socketDescriptor] = handler;
    ClientInfo info;
    info.socketDescriptor = socketDescriptor;
    info.ipAddress = handler->peerAddress();
    info.connectedAt = QDateTime::currentDateTime();
    m_clientInfo[socketDescriptor] = info;

    emit clientConnected(socketDescriptor, handler->peerAddress());
}

void Server::connectToClientSignals(ClientHandler* handler) {
    if (!handler) return;

    connect(handler, &ClientHandler::requestReceived,
            this, [this](const QString& request) {
                m_totalRequests.fetchAndAddOrdered(1);
                emit requestReceived(request);
            });

    connect(handler, &ClientHandler::responseSent,
            this, [this](const QString& response) {
                m_totalResponses.fetchAndAddOrdered(1);
                emit responseSent(response);
            });

    connect(handler, &ClientHandler::clientError,
            this, [this](const QString& error) {
                emit errorOccurred(error);
            });

    connect(handler, &ClientHandler::disconnected,
            this, [this, handler]() {
                qintptr descriptor = handler->m_socketDescriptor;
                m_clients.remove(descriptor);
                m_clientInfo.remove(descriptor);
                emit clientDisconnected(descriptor);
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


// Server.cpp

// =============================================
// ===== getOnlineUserCount =====
// =============================================

int Server::getOnlineUserCount() const
{
    // تعداد کلاینت‌های متصل را برمی‌گرداند
    return m_clients.size();
}



void Server::updateClientUsername(qintptr socketDescriptor, const QString& username) {
    auto it = m_clientInfo.find(socketDescriptor);
    if (it != m_clientInfo.end()) {
        it.value().username = username;
    }
}


QVariantMap Server::getTrafficStats() const {
    QVariantMap data;
    data["totalRequests"]     = static_cast<qlonglong>(m_totalRequests.loadAcquire());
    data["totalResponses"]    = static_cast<qlonglong>(m_totalResponses.loadAcquire());
    data["activeConnections"] = m_clients.size();
    return data;
}


QVariantList Server::getConnectedClientsInfo() const {
    QVariantList list;
    for (auto it = m_clientInfo.constBegin(); it != m_clientInfo.constEnd(); ++it) {
        const ClientInfo &info = it.value();
        qint64 secondsConnected = info.connectedAt.secsTo(QDateTime::currentDateTime());

        QVariantMap m;
        m["socketId"]     = static_cast<qlonglong>(info.socketDescriptor);
        m["ipAddress"]    = info.ipAddress;
        m["username"]     = info.username.isEmpty() ? QStringLiteral("(not logged in)") : info.username;
        m["connectedAt"]  = info.connectedAt.toString(Qt::ISODate);
        m["connectedFor"] = QString("%1:%2:%3")
                                .arg(secondsConnected / 3600, 2, 10, QChar('0'))
                                .arg((secondsConnected % 3600) / 60, 2, 10, QChar('0'))
                                .arg(secondsConnected % 60, 2, 10, QChar('0'));
        list.append(m);
    }
    return list;
}



QVariantMap Server::getResourceUsage() const {
    if (!m_resourceMonitor) {
        const_cast<Server*>(this)->m_resourceMonitor = new ServerResourceMonitor();
    }
    ResourceUsage usage = m_resourceMonitor->sample();

    QVariantMap data;
    data["available"] = usage.available;
    if (usage.available) {
        data["cpuPercent"] = usage.cpuPercent;
        data["ramUsedKB"]  = static_cast<qlonglong>(usage.ramUsedKB);
        data["ramTotalKB"] = static_cast<qlonglong>(usage.ramTotalKB);
        data["ramPercent"] = usage.ramPercent;
    }
    return data;
}
QString Server::getUptimeString() const
{
    if (!isRunning() || m_startTime.isNull()) {
        return "00:00:00";
    }

    qint64 uptimeSeconds = m_startTime.secsTo(QDateTime::currentDateTime());

    int hours = uptimeSeconds / 3600;
    int minutes = (uptimeSeconds % 3600) / 60;
    int seconds = uptimeSeconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}


void Server::broadcastToAll(const Response& response)
{
    qDebug() << "📢 Broadcasting to all clients:" << response.getMessage();

    for (ClientHandler* client : m_clients) {
        if (client) {

            client->sendResponse(response);
        }
    }
}


// Server.cpp
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>

// =============================================
// ===== scheduleRestart =====
// =============================================

void Server::scheduleRestart(int delayMs)
{
    if (m_restartPending) {
        qWarning() << "⚠️ Restart already pending!";
        return;
    }

    if (!isRunning()) {
        qWarning() << "⚠️ Server is not running! Cannot restart.";
        return;
    }

    // ذخیره پورت فعلی
    m_currentPort = serverPort();

    // ایجاد تایمر اگر وجود ندارد
    if (!m_restartTimer) {
        m_restartTimer = new QTimer(this);
        connect(m_restartTimer, &QTimer::timeout, this, &Server::performRestart);
    }

    m_restartPending = true;
    m_restartTimer->start(delayMs);

    qDebug() << "🔄 Server restart scheduled in" << delayMs << "ms";
    emit systemEvent(QString("Server restart scheduled in %1 ms").arg(delayMs));
}

// =============================================
// ===== cancelRestart =====
// =============================================

void Server::cancelRestart()
{
    if (m_restartTimer) {
        m_restartTimer->stop();
    }
    m_restartPending = false;

    qDebug() << "🔄 Server restart cancelled";
    emit systemEvent("Server restart cancelled");
}

// =============================================
// ===== performRestart =====
// =============================================

void Server::performRestart()
{
    m_restartPending = false;

    if (m_restartTimer) {
        m_restartTimer->stop();
    }

    qDebug() << "🔄 Performing server restart...";
    emit systemEvent("Server restarting...");

    // 1. توقف سرور فعلی
    stop();

    // 2. راه‌اندازی مجدد با پورت قبلی
    QTimer::singleShot(500, this, [this]() {
        if (!start(m_currentPort)) {
            qCritical() << "❌ Server failed to restart on port" << m_currentPort;
            emit systemEvent("Server restart failed!");
        } else {
            qDebug() << "✅ Server restarted successfully on port" << m_currentPort;
            emit systemEvent(QString("Server restarted successfully on port %1").arg(m_currentPort));
        }
    });
}




QString ClientHandler::getSessionUsername() const {
    QMutexLocker locker(&m_sessionMutex);
    return m_sessionUsername;
}

QVariantMap ClientHandler::getServerResourceUsage() const {
    if (Server* server = qobject_cast<Server*>(parent()))
        return server->getResourceUsage();
    return QVariantMap();
}

QVariantList ClientHandler::getConnectedClientsInfo() const {
    if (Server* server = qobject_cast<Server*>(parent()))
        return server->getConnectedClientsInfo();
    return QVariantList();
}


QVariantMap ClientHandler::getTrafficStats() const {
    if (Server* server = qobject_cast<Server*>(parent()))
        return server->getTrafficStats();
    return QVariantMap();
}



