// ClientHandler.h
#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QTcpSocket>
#include <QObject>

class RequestParser;
class Response;
class AuthService;
class BookService;
class UserService;
class PurchaseService;
class ReviewService;

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
                           QObject *parent = nullptr);
    ~ClientHandler();

signals:
    void disconnected();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket* m_socket;
    qintptr m_socketDescriptor;

    // سرویس‌ها
    AuthService* m_authService;
    BookService* m_bookService;
    UserService* m_userService;
    PurchaseService* m_purchaseService;
    ReviewService* m_reviewService;

    RequestParser* m_parser;

    void sendResponse(const QString& response);
    void handleRequest(const QString& request);
};

#endif // CLIENTHANDLER_H