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
signals:
    void disconnected();
    void responseReady(const Response& response);


private slots:
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError socketError);
     void onResponseReady(const Response& response);

private:
    QTcpSocket* m_socket = nullptr;
    qintptr m_socketDescriptor;

    AuthService* m_authService;
    BookService* m_bookService;
    UserService* m_userService;
    PurchaseService* m_purchaseService;
    ReviewService* m_reviewService;
    CartService* m_cartService;
    PublisherService* m_publisherService;
    AdminService* m_adminService;

    RequestParser* m_parser = nullptr;

    void sendResponse(const QString& response);
    void sendResponse(const Response& response);
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