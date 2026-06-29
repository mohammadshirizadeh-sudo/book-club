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

#include "../Shared/Book.h"
#include "../Shared/User.h"
#include "../Shared/Purchase.h"
#include "../Shared/Review.h"
#include "../Shared/CartItem.h"

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

signals:
    void disconnected();

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    // ===== Socket =====
    QTcpSocket* m_socket;
    qintptr m_socketDescriptor;

    // ===== Services =====
    AuthService* m_authService;
    BookService* m_bookService;
    UserService* m_userService;
    PurchaseService* m_purchaseService;
    ReviewService* m_reviewService;
    CartService* m_cartService;
    PublisherService* m_publisherService;
    AdminService* m_adminService;

    // ===== Parser =====
    RequestParser* m_parser;

    // ===== Core Methods =====
    void sendResponse(const QString& response);
    void sendResponse(const Response& response);
    void handleRequest(const QString& requestData);

    // ===== Command Handlers =====
    Response handleLogin(const Request& request);
    Response handleRegister(const Request& request);
    Response handleLogout(const Request& request);
    Response handleResetPassword(const Request& request);

    Response handleGetProfile(const Request& request);
    Response handleUpdateProfile(const Request& request);
    Response handleChangePassword(const Request& request);
    Response handleUpdateFavoriteGenres(const Request& request);

    Response handleSearchBooks(const Request& request);
    Response handleGetBookById(const Request& request);
    Response handleGetBooksByGenre(const Request& request);
    Response handleGetPopularBooks(const Request& request);
    Response handleGetNewBooks(const Request& request);
    Response handleGetFreeBooks(const Request& request);
    Response handleGetRecommendedBooks(const Request& request);

    Response handleAddToCart(const Request& request);
    Response handleRemoveFromCart(const Request& request);
    Response handleUpdateCartQuantity(const Request& request);
    Response handleGetCart(const Request& request);
    Response handleClearCart(const Request& request);

    Response handleCheckout(const Request& request);
    Response handleGetPurchaseHistory(const Request& request);
    Response handleGetPurchaseById(const Request& request);

    Response handleAddReview(const Request& request);
    Response handleEditReview(const Request& request);
    Response handleDeleteReview(const Request& request);
    Response handleGetReviewsForBook(const Request& request);
    Response handleGetAverageRating(const Request& request);

    Response handleAddBook(const Request& request);
    Response handleEditBook(const Request& request);
    Response handleDeactivateBook(const Request& request);
    Response handleReactivateBook(const Request& request);
    Response handleGetPublisherBooks(const Request& request);
    Response handleGetPublisherStats(const Request& request);

    Response handleBlockUser(const Request& request);
    Response handleUnblockUser(const Request& request);
    Response handleDeleteUser(const Request& request);
    Response handleGetAllUsers(const Request& request);
    Response handleGetBlockedUsers(const Request& request);
    Response handleAdminDeleteBook(const Request& request);
    Response handleAdminDeleteReview(const Request& request);
    Response handleGetSystemStats(const Request& request);

    // ===== Helper Methods =====
    QJsonObject bookToJson(Book* book);
    QJsonObject userToJson(User* user);
    QJsonObject purchaseToJson(Purchase* purchase);
    QJsonObject reviewToJson(Review* review);
    QJsonObject cartItemToJson(const CartItem& item);
};

#endif // CLIENTHANDLER_H