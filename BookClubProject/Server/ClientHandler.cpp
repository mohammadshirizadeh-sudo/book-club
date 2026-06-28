// ClientHandler.cpp
#include "ClientHandler.h"
#include "RequestParser.h"
#include "Request.h"
#include "Response.h"
#include "Services/AuthService.h"
#include "Services/BookService.h"
#include "Services/UserService.h"
#include "Services/PurchaseService.h"
#include "Services/ReviewService.h"
#include <QDebug>

ClientHandler::ClientHandler(qintptr socketDescriptor,
                             AuthService* authService,
                             BookService* bookService,
                             UserService* userService,
                             PurchaseService* purchaseService,
                             ReviewService* reviewService,
                             QObject *parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_authService(authService)
    , m_bookService(bookService)
    , m_userService(userService)
    , m_purchaseService(purchaseService)
    , m_reviewService(reviewService)
{
    m_socket = new QTcpSocket(this);

    if (!m_socket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor:" << socketDescriptor;
        deleteLater();
        return;
    }

    m_parser = new RequestParser(this);

    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);

    qDebug() << "ClientHandler created for socket:" << socketDescriptor;
}

ClientHandler::~ClientHandler()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
    }
    qDebug() << "ClientHandler destroyed for socket:" << m_socketDescriptor;
}

void ClientHandler::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString requestData = QString::fromUtf8(data).trimmed();

    if (!requestData.isEmpty()) {
        qDebug() << "Received request from client:" << requestData;
        handleRequest(requestData);
    }
}

void ClientHandler::onDisconnected()
{
    qDebug() << "Client disconnected:" << m_socketDescriptor;
    emit disconnected();
    deleteLater();
}

void ClientHandler::handleRequest(const QString& requestData)
{
    // 1. Parse request → تبدیل به Request object
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponse(Response::error("Invalid request format").toJson());
        return;
    }

    // 2. Execute command → اجرای سرویس مناسب
    Response response;

    switch (request.getCommand()) {
    case Command::Login:
        response = handleLogin(request);
        break;

    case Command::Register:
        response = handleRegister(request);
        break;

    case Command::SearchBooks:
        response = handleSearchBooks(request);
        break;

    case Command::AddToCart:
        response = handleAddToCart(request);
        break;

    case Command::Checkout:
        response = handleCheckout(request);
        break;

    case Command::AddReview:
        response = handleAddReview(request);
        break;

        // ... بقیه Commands ...

    default:
        response = Response::error("Unknown command");
        break;
    }

    // 3. Send response → ارسال پاسخ به کلاینت
    sendResponse(response.toJson());
}

// ===== Command Handlers =====

Response ClientHandler::handleLogin(const Request& request)
{
    QString username = request.getParam("username").toString();
    QString password = request.getParam("password").toString();

    User* user = m_authService->login(username, password);
    if (user) {
        return Response::success({
            {"userId", user->getId()},
            {"username", user->getUsername()},
            {"role", user->getRoleString()}
        });
    } else {
        return Response::error("Invalid username or password");
    }
}

Response ClientHandler::handleRegister(const Request& request)
{
    QString username = request.getParam("username").toString();
    QString email = request.getParam("email").toString();
    QString password = request.getParam("password").toString();
    QString role = request.getParam("role").toString();

    UserRole userRole = UserRole::User;
    if (role == "Publisher") userRole = UserRole::Publisher;
    else if (role == "Admin") userRole = UserRole::Admin;

    User* user = m_authService->registerUser(username, email, password, userRole);
    if (user) {
        return Response::success({
            {"userId", user->getId()},
            {"username", user->getUsername()}
        });
    } else {
        return Response::error("Registration failed");
    }
}

Response ClientHandler::handleSearchBooks(const Request& request)
{
    QString keyword = request.getParam("keyword").toString();
    QVector<Book*> books = m_bookService->searchBooks(keyword);

    QJsonArray booksArray;
    for (Book* book : books) {
        booksArray.append(bookToJson(book));
    }

    return Response::success({
        {"books", booksArray},
        {"count", books.size()}
    });
}

Response ClientHandler::handleAddToCart(const Request& request)
{
    int userId = request.getParam("userId").toInt();
    int bookId = request.getParam("bookId").toInt();
    int quantity = request.getParam("quantity").toInt(1);

    // نیاز به CartService داریم
    // if (m_cartService->addToCart(userId, bookId, quantity)) {
    //     return Response::success("Added to cart");
    // } else {
    //     return Response::error("Failed to add to cart");
    // }

    return Response::success("Added to cart (placeholder)");
}

Response ClientHandler::handleCheckout(const Request& request)
{
    int userId = request.getParam("userId").toInt();

    // if (m_purchaseService->checkout(userId)) {
    //     return Response::success("Purchase successful");
    // } else {
    //     return Response::error("Purchase failed");
    // }

    return Response::success("Purchase successful (placeholder)");
}

Response ClientHandler::handleAddReview(const Request& request)
{
    int userId = request.getParam("userId").toInt();
    int bookId = request.getParam("bookId").toInt();
    QString text = request.getParam("text").toString();
    int rating = request.getParam("rating").toInt();

    if (m_reviewService->addReview(userId, bookId, text, rating)) {
        return Response::success("Review added");
    } else {
        return Response::error("Failed to add review");
    }
}

// ===== Helper =====

QJsonObject ClientHandler::bookToJson(Book* book)
{
    QJsonObject obj;
    obj["bookId"] = book->getBookId();
    obj["title"] = book->getTitle();
    obj["author"] = book->getAuthor();
    obj["genre"] = book->getGenre();
    obj["price"] = book->getPrice();
    obj["discountPercent"] = book->getDiscountPercent();
    obj["finalPrice"] = book->getFinalPrice();
    obj["averageRating"] = book->getAverageRating();
    return obj;
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