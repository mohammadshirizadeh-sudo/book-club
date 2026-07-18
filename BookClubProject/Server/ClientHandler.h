// ClientHandler.h
#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QTcpSocket>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include "RequestParser.h"
#include "Request.h"
#include "Response.h"
#include "../Services/AuthService.h"
#include "../Services/BookService.h"
#include "../Services/UserService.h"
#include "../Services/PurchaseService.h"
#include "../Services/ReviewService.h"
#include "../Services/CartService.h"
#include "../Services/PublisherService.h"
#include "../Services/AdminService.h"

#include "../Shared/Book.h"
#include "../Shared/User.h"
#include "../Shared/Purchase.h"
#include "../Shared/Review.h"
#include "../Shared/CartItem.h"
#include "Commands.h"
#include "CommandFactory.h"


class RequestParser;
class Request;
class Response;
class AuthService;
class BookService;
class UserService;
class PurchaseService;
class ReviewService;
class CartService;
class PublisherService;
class AdminService;
class Book;

class ClientHandler : public QObject
{
    Q_OBJECT

public:
    explicit ClientHandler(qintptr socketDescriptor,
                           AuthService* authService,
                           BookService* bookService,
                           UserService* userService,
                           PurchaseService* purchaseService,
                           ReviewService* reviewService,
                           CartService* cartService,
                           PublisherService* publisherService,
                           AdminService* adminService,
                           QObject *parent = nullptr);
    ~ClientHandler();


    void setSession(int userId, UserRole role);
    void disconnectFromClient();


    bool isValidSocket() const
    {
        return m_socket != nullptr;
    }

    QString peerAddress() const
    {
        return m_socket
                   ? m_socket->peerAddress().toString()
                   : QString();
    }


    qintptr m_socketDescriptor;
signals:
    void disconnected();
    void responseReady(const Response& response);

    void requestReceived(const QString& request);
    void responseSent(const QString& response);
    void clientError(const QString& error);



private slots:
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
     void onResponseReady(const Response& response);

private:
    QTcpSocket* m_socket = nullptr;


    QAtomicInt m_pendingTasks{0};
    QAtomicInt m_isDestroying{0};


    mutable QMutex m_sessionMutex;

    AuthService* m_authService;
    BookService* m_bookService;
    UserService* m_userService;
    PurchaseService* m_purchaseService;
    ReviewService* m_reviewService;
    CartService* m_cartService;
    PublisherService* m_publisherService;
    AdminService* m_adminService;
    NotificationService* m_notificationService;
    LibraryService* m_libraryService;

    RequestParser* m_parser = nullptr;

    void sendResponse(const QString& response);

    QByteArray m_recvBuffer;
private slots:

    void sendResponse(const Response& response);
private:

    void handleRequest(const QString& requestData);

    void handleRequestSync(const QString& requestData);
    void sendResponseSync(const Response& response);


    void processRequest(const QString& requestData);
private:
    int m_sessionUserId = -1;
    UserRole m_sessionRole = UserRole::User;
    bool m_isAuthenticated = false;





};

#endif // CLIENTHANDLER_H