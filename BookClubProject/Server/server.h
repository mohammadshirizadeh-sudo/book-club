// Server.h (نسخه بهینه)
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QMap>

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

    void initServices();
    void cleanupServices();
};

#endif // SERVER_H