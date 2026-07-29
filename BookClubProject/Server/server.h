// Server.h (نسخه بهینه)
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QMap>
#include <QDateTime>
#include <QMutex>
#include "../Server/Response.h"
#include <QTimer>
#include "ServerResourceMonitor.h"
#include <QAtomicInteger>


struct ClientInfo {
    qintptr socketDescriptor = 0;
    QString ipAddress;
    QString username;
    QDateTime connectedAt;
};

class ClientHandler;
class UserRepository;
class BookRepository;
class ReviewRepository;
class PurchaseRepository;
class LibraryRepository;

class AuthService;
class UserService;
class BookService;
class CartService;
class PurchaseService;
class ReviewService;
class PublisherService;
class AdminService;
class NotificationService;
class LibraryService;
class ReadingSessionService;

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool start(quint16 port);
    void stop();

    bool isRunning() const { return isListening(); }

    void stopServer();


    QList<ClientHandler*> getClients() const {
        return m_clients.values();
    }

    void connectToClientSignals(ClientHandler* handler);
    bool startServer(quint16 port);


    AdminService* getAdminService() const
    {
        return m_adminService;
    }

    ReadingSessionService* getReadingSessionService() const
    {
        return m_readingSessionService;
    }


    int getOnlineUserCount() const;

    QString getUptimeString() const;
    void broadcastToAll(const Response& response);

    // Targeted push: send a Response to the ClientHandler currently
    // registered for this userId, if any. Silent no-op if that user isn't
    // connected right now (e.g. offline session participant) - callers
    // should treat their own periodic sync/poll as the fallback for that.
    void sendToUser(int userId, const Response& response);

    // Registry maintained by ClientHandler::setSession(...) once a
    // connection is authenticated/identified, and cleared on disconnect.
    void registerUserHandler(int userId, ClientHandler* handler);
    void unregisterUserHandler(int userId);


    void scheduleRestart(int delayMs = 1000);
    void cancelRestart();
    bool isRestartPending() const { return m_restartPending; }


    QVariantMap  getResourceUsage() const;
    QVariantList getConnectedClientsInfo() const;
    QVariantMap  getTrafficStats() const;
    void updateClientUsername(qintptr socketDescriptor, const QString& username);



private slots:
    void performRestart();

signals:
    void clientConnected(qintptr socketDescriptor, const QString& ipAddress);

    void clientDisconnected(qintptr socketDescriptor);
    void requestReceived(const QString& request);
    void responseSent(const QString& response);
    void errorOccurred(const QString& error);
    void systemEvent(const QString& event);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMap<qintptr, ClientHandler*> m_clients;

    // ===== Repositories =====
    UserRepository* m_userRepo;
    BookRepository* m_bookRepo;
    ReviewRepository* m_reviewRepo;
    PurchaseRepository* m_purchaseRepo;
    LibraryRepository* m_libraryRepo;

    // ===== Services =====
    AuthService* m_authService;
    UserService* m_userService;
    BookService* m_bookService;
    CartService* m_cartService;
    PurchaseService* m_purchaseService;
    ReviewService* m_reviewService;
    PublisherService* m_publisherService;
    AdminService* m_adminService;
    NotificationService* m_notifService;
    LibraryService* m_libraryService;
    ReadingSessionService* m_readingSessionService = nullptr;
    QDateTime m_startTime;

    void initServices();
    void cleanupServices();


    QTimer* m_restartTimer = nullptr;
    bool m_restartPending = false;
    quint16 m_currentPort = 0;


    QMap<qintptr, ClientInfo> m_clientInfo;
    mutable ServerResourceMonitor* m_resourceMonitor = nullptr;
    QAtomicInteger<qint64> m_totalRequests{0};
    QAtomicInteger<qint64> m_totalResponses{0};

    // userId -> currently-connected ClientHandler, for targeted push
    // (GroupReading page-sync/chat/participant events). Guarded separately
    // from m_clients/m_clientInfo since it's written from ClientHandler's
    // request-handling threads (via setSession), not just the accept thread.
    QMap<int, ClientHandler*> m_userIdToHandler;
    mutable QMutex m_userRegistryMutex;
};

#endif // SERVER_H
