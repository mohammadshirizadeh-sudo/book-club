// ClientHandler.cpp

#include <QDebug>
#include "ClientHandler.h"



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

// ===== Slots =====

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

// ===== Core Methods =====

void ClientHandler::handleRequest(const QString& requestData)
{
    // 1. Parse request
    Request request = m_parser->parse(requestData);
    if (!request.isValid()) {
        sendResponse(Response::error("Invalid request format"));
        return;
    }

    // 2. Execute command
    Response response;

    switch (request.getCommandType()) {
    // Auth
    case CommandType::Login:        response = handleLogin(request); break;
    case CommandType::Register:     response = handleRegister(request); break;
    case CommandType::Logout:       response = handleLogout(request); break;
    case CommandType::ResetPassword: response = handleResetPassword(request); break;

    // User
    case CommandType::GetProfile:   response = handleGetProfile(request); break;
    case CommandType::UpdateProfile: response = handleUpdateProfile(request); break;
    case CommandType::ChangePassword: response = handleChangePassword(request); break;
    case CommandType::UpdateFavoriteGenres: response = handleUpdateFavoriteGenres(request); break;

    // Book
    case CommandType::SearchBooks:  response = handleSearchBooks(request); break;
    case CommandType::GetBookById:  response = handleGetBookById(request); break;
    case CommandType::GetBooksByGenre: response = handleGetBooksByGenre(request); break;
    case CommandType::GetPopularBooks: response = handleGetPopularBooks(request); break;
    case CommandType::GetNewBooks:  response = handleGetNewBooks(request); break;
    case CommandType::GetFreeBooks: response = handleGetFreeBooks(request); break;
    case CommandType::GetRecommendedBooks: response = handleGetRecommendedBooks(request); break;

    // Cart
    case CommandType::AddToCart:    response = handleAddToCart(request); break;
    case CommandType::RemoveFromCart: response = handleRemoveFromCart(request); break;
    case CommandType::UpdateCartQuantity: response = handleUpdateCartQuantity(request); break;
    case CommandType::GetCart:      response = handleGetCart(request); break;
    case CommandType::ClearCart:    response = handleClearCart(request); break;

    // Purchase
    case CommandType::Checkout:     response = handleCheckout(request); break;
    case CommandType::GetPurchaseHistory: response = handleGetPurchaseHistory(request); break;
    case CommandType::GetPurchaseById: response = handleGetPurchaseById(request); break;

    // Review
    case CommandType::AddReview:    response = handleAddReview(request); break;
    case CommandType::EditReview:   response = handleEditReview(request); break;
    case CommandType::DeleteReview: response = handleDeleteReview(request); break;
    case CommandType::GetReviewsForBook: response = handleGetReviewsForBook(request); break;
    case CommandType::GetAverageRating: response = handleGetAverageRating(request); break;

    // Publisher
    case CommandType::AddBook:      response = handleAddBook(request); break;
    case CommandType::EditBook:     response = handleEditBook(request); break;
    case CommandType::DeactivateBook: response = handleDeactivateBook(request); break;
    case CommandType::ReactivateBook: response = handleReactivateBook(request); break;
    case CommandType::GetPublisherBooks: response = handleGetPublisherBooks(request); break;
    case CommandType::GetPublisherStats: response = handleGetPublisherStats(request); break;

    // Admin
    case CommandType::BlockUser:    response = handleBlockUser(request); break;
    case CommandType::UnblockUser:  response = handleUnblockUser(request); break;
    case CommandType::DeleteUser:   response = handleDeleteUser(request); break;
    case CommandType::GetAllUsers:  response = handleGetAllUsers(request); break;
    case CommandType::GetBlockedUsers: response = handleGetBlockedUsers(request); break;
    case CommandType::DeleteBook:   response = handleAdminDeleteBook(request); break;
    case CommandType::GetSystemStats: response = handleGetSystemStats(request); break;

    default:
        response = Response::error("Unknown command: " + request.getCommandTypeString());
        break;
    }

    // 3. Send response
    sendResponse(response);
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

// =============================================
// ===== Auth Command Handlers =====
// =============================================

Response ClientHandler::handleLogin(const Request& request)
{
    QString username = request.getParamString("username");
    QString password = request.getParamString("password");

    User* user = m_authService->login(username, password);
    if (user) {
        QVariantMap data;
        data["userId"] = user->getId();
        data["username"] = user->getUsername();
        data["role"] = user->getRoleString();
        return Response::success("Login successful", data);
    }

    return Response::error("Invalid username or password");
}

Response ClientHandler::handleRegister(const Request& request)
{
    QString username = request.getParamString("username");
    QString email = request.getParamString("email");
    QString password = request.getParamString("password");
    QString roleStr = request.getParamString("role", "User");

    UserRole role = UserRole::User;
    if (roleStr == "Publisher") role = UserRole::Publisher;
    else if (roleStr == "Admin") role = UserRole::Admin;

    User* user = m_authService->registerUser(username, email, password, role);
    if (user) {
        QVariantMap data;
        data["userId"] = user->getId();
        data["username"] = user->getUsername();
        return Response::success("Registration successful", data);
    }

    return Response::error("Registration failed");
}

Response ClientHandler::handleLogout(const Request& request)
{
    int userId = request.getParamInt("userId");
    m_authService->logout();
    return Response::success("Logout successful");
}

Response ClientHandler::handleResetPassword(const Request& request)
{
    QString email = request.getParamString("email");
    if (m_authService->requestPasswordReset(email)) {
        return Response::success("Password reset email sent");
    }
    return Response::error("Failed to reset password");
}

// =============================================
// ===== User Command Handlers =====
// =============================================

Response ClientHandler::handleGetProfile(const Request& request)
{
    int userId = request.getParamInt("userId");
    User* user = m_userService->getProfile(userId);
    if (user) {
        QVariantMap data;
        data["id"] = user->getId();
        data["username"] = user->getUsername();
        data["email"] = user->getEmail();
        data["fullName"] = user->getFullname();
        data["role"] = user->getRoleString();
        data["status"] = static_cast<int>(user->getStatus());
        data["favoriteGenres"] = QVariant::fromValue(user->getFavouriteGenre());
        return Response::success(data);
    }
    return Response::error("User not found");
}

Response ClientHandler::handleUpdateProfile(const Request& request)
{
    int userId = request.getParamInt("userId");
    QString email = request.getParamString("email");
    QString fullName = request.getParamString("fullName");
    QStringList genres = request.getParamStringList("favoriteGenres");
    QVector<QString> genreVector = genres.toVector();

    if (m_userService->updateProfile(userId, email, fullName, genreVector)) {
        return Response::success("Profile updated successfully");
    }
    return Response::error("Failed to update profile");
}

Response ClientHandler::handleChangePassword(const Request& request)
{
    int userId = request.getParamInt("userId");
    QString oldPassword = request.getParamString("oldPassword");
    QString newPassword = request.getParamString("newPassword");

    if (m_userService->changePassword(userId, oldPassword, newPassword)) {
        return Response::success("Password changed successfully");
    }
    return Response::error("Failed to change password");
}

Response ClientHandler::handleUpdateFavoriteGenres(const Request& request)
{
    int userId = request.getParamInt("userId");
    QStringList genres = request.getParamStringList("genres");
    QVector<QString> genreVector = genres.toVector();

    if (m_userService->updateFavoriteGenres(userId, genreVector)) {
        return Response::success("Favorite genres updated");
    }
    return Response::error("Failed to update favorite genres");
}

// =============================================
// ===== Book Command Handlers =====
// =============================================

Response ClientHandler::handleSearchBooks(const Request& request)
{
    QString keyword = request.getParamString("keyword");
    QVector<Book*> books = m_bookService->searchBooks(keyword);

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetBookById(const Request& request)
{
    int bookId = request.getParamInt("bookId");
    Book* book = m_bookService->getBookById(bookId);
    if (book) {
        return Response::success(bookToJson(book).toVariantMap());
    }
    return Response::error("Book not found");
}

Response ClientHandler::handleGetBooksByGenre(const Request& request)
{
    QString genre = request.getParamString("genre");
    QVector<Book*> books = m_bookService->getBooksByGenre(genre);

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetPopularBooks(const Request& request)
{
    int limit = request.getParamInt("limit", 10);
    QVector<Book*> books = m_bookService->getPopularBooks(limit);

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetNewBooks(const Request& request)
{
    int limit = request.getParamInt("limit", 10);
    QVector<Book*> books = m_bookService->getNewBooks(limit);

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetFreeBooks(const Request& request)
{
    QVector<Book*> books = m_bookService->getFreeBooks();

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetRecommendedBooks(const Request& request)
{
    int userId = request.getParamInt("userId");
    int limit = request.getParamInt("limit", 10);

    User* user = m_userService->getProfile(userId);
    if (!user) {
        return Response::error("User not found");
    }

    QVector<Book*> books = m_bookService->getRecommendedBooks(
        user->getFavouriteGenre(),
        limit
        );

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// =============================================
// ===== Cart Command Handlers =====
// =============================================

Response ClientHandler::handleAddToCart(const Request& request)
{
    int userId = request.getParamInt("userId");
    int bookId = request.getParamInt("bookId");
    int quantity = request.getParamInt("quantity", 1);

    if (m_cartService->addToCart(userId, bookId, quantity)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Added to cart", data);
    }
    return Response::error("Failed to add to cart");
}

Response ClientHandler::handleRemoveFromCart(const Request& request)
{
    int userId = request.getParamInt("userId");
    int bookId = request.getParamInt("bookId");

    if (m_cartService->removeFromCart(userId, bookId)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Removed from cart", data);
    }
    return Response::error("Failed to remove from cart");
}

Response ClientHandler::handleUpdateCartQuantity(const Request& request)
{
    int userId = request.getParamInt("userId");
    int bookId = request.getParamInt("bookId");
    int quantity = request.getParamInt("quantity");

    if (m_cartService->updateQuantity(userId, bookId, quantity)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Cart updated", data);
    }
    return Response::error("Failed to update cart");
}

Response ClientHandler::handleGetCart(const Request& request)
{
    int userId = request.getParamInt("userId");

    QVariantMap data;
    data["items"] = QVariant::fromValue(m_cartService->getCartItems(userId));
    data["totalItems"] = m_cartService->getTotalItemCount(userId);
    data["totalPrice"] = m_cartService->getTotalPrice(userId);
    data["totalDiscount"] = m_cartService->getTotalDiscount(userId);
    data["finalPrice"] = m_cartService->getFinalPrice(userId);
    data["isEmpty"] = m_cartService->isEmpty(userId);

    return Response::success(data);
}

Response ClientHandler::handleClearCart(const Request& request)
{
    int userId = request.getParamInt("userId");
    m_cartService->clearCart(userId);
    return Response::success("Cart cleared");
}

// =============================================
// ===== Purchase Command Handlers =====
// =============================================

Response ClientHandler::handleCheckout(const Request& request)
{
    int userId = request.getParamInt("userId");

    Purchase* purchase = m_purchaseService->checkout(userId);
    if (purchase) {
        QVariantMap data;
        data["purchaseId"] = purchase->getPurchaseId();
        data["finalPrice"] = purchase->getFinalPrice();
        data["totalItems"] = purchase->getTotalItemCount();
        data["status"] = purchase->getStatusString();
        return Response::success("Purchase successful", data);
    }
    return Response::error("Purchase failed");
}

Response ClientHandler::handleGetPurchaseHistory(const Request& request)
{
    int userId = request.getParamInt("userId");
    QVector<Purchase*> purchases = m_purchaseService->getPurchaseHistory(userId);

    QVariantList purchaseList;
    for (Purchase* purchase : purchases) {
        purchaseList.append(purchaseToJson(purchase).toVariantMap());
    }

    QVariantMap data;
    data["purchases"] = purchaseList;
    data["count"] = purchaseList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetPurchaseById(const Request& request)
{
    int purchaseId = request.getParamInt("purchaseId");
    Purchase* purchase = m_purchaseService->getPurchaseById(purchaseId);
    if (purchase) {
        return Response::success(purchaseToJson(purchase).toVariantMap());
    }
    return Response::error("Purchase not found");
}

// =============================================
// ===== Review Command Handlers =====
// =============================================

Response ClientHandler::handleAddReview(const Request& request)
{
    int userId = request.getParamInt("userId");
    int bookId = request.getParamInt("bookId");
    QString text = request.getParamString("text");
    int rating = request.getParamInt("rating");

    if (m_reviewService->addReview(userId, bookId, text, rating)) {
        QVariantMap data;
        data["averageRating"] = m_reviewService->getAverageRating(bookId);
        return Response::success("Review added", data);
    }
    return Response::error("Failed to add review");
}

Response ClientHandler::handleEditReview(const Request& request)
{
    int reviewId = request.getParamInt("reviewId");
    int userId = request.getParamInt("userId");
    QString text = request.getParamString("text");
    int rating = request.getParamInt("rating");

    if (m_reviewService->editReview(reviewId, userId, text, rating)) {
        return Response::success("Review updated");
    }
    return Response::error("Failed to update review");
}

Response ClientHandler::handleDeleteReview(const Request& request)
{
    int reviewId = request.getParamInt("reviewId");
    int userId = request.getParamInt("userId");

    if (m_reviewService->deleteReview(reviewId, userId)) {
        return Response::success("Review deleted");
    }
    return Response::error("Failed to delete review");
}

Response ClientHandler::handleGetReviewsForBook(const Request& request)
{
    int bookId = request.getParamInt("bookId");
    QVector<Review*> reviews = m_reviewService->getReviewsForBook(bookId);

    QVariantList reviewList;
    for (Review* review : reviews) {
        reviewList.append(reviewToJson(review).toVariantMap());
    }

    QVariantMap data;
    data["reviews"] = reviewList;
    data["count"] = reviewList.size();
    data["averageRating"] = m_reviewService->getAverageRating(bookId);
    return Response::success(data);
}

Response ClientHandler::handleGetAverageRating(const Request& request)
{
    int bookId = request.getParamInt("bookId");
    double avg = m_reviewService->getAverageRating(bookId);

    QVariantMap data;
    data["bookId"] = bookId;
    data["averageRating"] = avg;
    return Response::success(data);
}

// =============================================
// ===== Publisher Command Handlers =====
// =============================================

Response ClientHandler::handleAddBook(const Request& request)
{
    int publisherId = request.getParamInt("publisherId");
    QString title = request.getParamString("title");
    QString author = request.getParamString("author");
    QString genre = request.getParamString("genre");
    QString description = request.getParamString("description");
    double price = request.getParamDouble("price");

    if (m_publisherService->addBook(publisherId, title, author, genre, description, price)) {
        return Response::success("Book added successfully");
    }
    return Response::error("Failed to add book");
}

Response ClientHandler::handleEditBook(const Request& request)
{
    int publisherId = request.getParamInt("publisherId");
    int bookId = request.getParamInt("bookId");
    QString title = request.getParamString("title");
    QString author = request.getParamString("author");
    QString genre = request.getParamString("genre");
    QString description = request.getParamString("description");
    double price = request.getParamDouble("price");
    double discount = request.getParamDouble("discount");

    if (m_publisherService->editBook(bookId, title, author, genre, description, price ,discount)) {
        return Response::success("Book updated successfully");
    }
    return Response::error("Failed to update book");
}

Response ClientHandler::handleDeactivateBook(const Request& request)
{

    int bookId = request.getParamInt("bookId");

    if (m_bookService->deactivateBook(bookId)) {
        return Response::success("Book deactivated");
    }
    return Response::error("Failed to deactivate book");
}

Response ClientHandler::handleReactivateBook(const Request& request)
{
    int publisherId = request.getParamInt("publisherId");
    int bookId = request.getParamInt("bookId");

    if (m_bookService->reactivateBook(bookId)) {
        return Response::success("Book reactivated");
    }
    return Response::error("Failed to reactivate book");
}

Response ClientHandler::handleGetPublisherBooks(const Request& request)
{
    int publisherId = request.getParamInt("publisherId");
    QVector<Book*> books = m_publisherService->getBooksByPublisher(publisherId);

    QVariantList bookList;
    for (Book* book : books) {
        bookList.append(bookToJson(book).toVariantMap());
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetPublisherStats(const Request& request)
{
    int publisherId = request.getParamInt("publisherId");
    QMap<QString, QVariant> stats = m_publisherService->getSalesStatistics(publisherId);

    QVariantMap data;
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        data[it.key()] = it.value();
    }
    return Response::success(data);
}

// =============================================
// ===== Admin Command Handlers =====
// =============================================

Response ClientHandler::handleBlockUser(const Request& request)
{
    int userId = request.getParamInt("userId");
    QString reason = request.getParamString("reason");

    if (m_adminService->blockUser(userId, reason))  {

        return Response::success("User blocked");
    }
    return Response::error("Failed to block user");
}


Response ClientHandler::handleUnblockUser(const Request& request)
{
    int userId = request.getParamInt("userId");

    if (m_adminService->unblockUser(userId)) {
        return Response::success("User unblocked");
    }
    return Response::error("Failed to unblock user");
}

Response ClientHandler::handleDeleteUser(const Request& request)
{
    int userId = request.getParamInt("userId");

    if (m_adminService->deleteUser(userId)) {
        return Response::success("User deleted");
    }
    return Response::error("Failed to delete user");
}

Response ClientHandler::handleGetAllUsers(const Request& request)
{
    QVector<User*> users = m_adminService->getAllUsers();

    QVariantList userList;
    for (User* user : users) {
        userList.append(userToJson(user).toVariantMap());
    }

    QVariantMap data;
    data["users"] = userList;
    data["count"] = userList.size();
    return Response::success(data);
}

Response ClientHandler::handleGetBlockedUsers(const Request& request)
{
    QVector<User*> users = m_adminService->getBlockedUsers();

    QVariantList userList;
    for (User* user : users) {
        userList.append(userToJson(user).toVariantMap());
    }

    QVariantMap data;
    data["users"] = userList;
    data["count"] = userList.size();
    return Response::success(data);
}

Response ClientHandler::handleAdminDeleteBook(const Request& request)
{
    int bookId = request.getParamInt("bookId");

    if (m_bookService->deleteBook(bookId)) {
        return Response::success("Book deleted by admin");
    }
    return Response::error("Failed to delete book");
}

Response ClientHandler::handleAdminDeleteReview(const Request& request)
{
    int reviewId = request.getParamInt("reviewId");
    int userId =request.getParamInt("userId");
    QString reason = request.getParamString("reason");

    if (m_reviewService->deleteReview(reviewId , userId)) {
        return Response::success("Review deleted by admin");
    }
    return Response::error("Failed to delete review");
}

Response ClientHandler::handleGetSystemStats(const Request& request)
{
    QMap<QString, QVariant> stats = m_adminService->getSystemStats();

    QVariantMap data;
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        data[it.key()] = it.value();
    }
    return Response::success(data);
}

// =============================================
// ===== Helper Methods =====
// =============================================

QJsonObject ClientHandler::bookToJson(Book* book)
{
    QJsonObject obj;
    obj["bookId"] = book->getBookId();
    obj["title"] = book->getTitle();
    obj["author"] = book->getAuthor();
    obj["genre"] = book->getGenre();
    obj["description"] = book->getDescription();
    obj["price"] = book->getPrice();
    obj["discountPercent"] = book->getDiscountPercent();
    obj["finalPrice"] = book->getFinalPrice();
    obj["averageRating"] = book->getAverageRating();
    obj["salesCount"] = book->getSalesCount();
    obj["isActive"] = book->getIsActive();
    obj["coverPath"] = book->getCoverPath();
    obj["pdfPath"] = book->getPdfPath();
    obj["publisherId"] = book->getPublisherId();
    return obj;
}

QJsonObject ClientHandler::userToJson(User* user)
{
    QJsonObject obj;
    obj["id"] = user->getId();
    obj["username"] = user->getUsername();
    obj["email"] = user->getEmail();
    obj["fullName"] = user->getFullname();
    obj["role"] = user->getRoleString();
    obj["status"] = static_cast<int>(user->getStatus());
    obj["isBlocked"] = user->isBlocked();
    return obj;
}

QJsonObject ClientHandler::purchaseToJson(Purchase* purchase)
{
    QJsonObject obj;
    obj["purchaseId"] = purchase->getPurchaseId();
    obj["userId"] = purchase->getUserId();
    obj["totalPrice"] = purchase->getTotalPrice();
    obj["discountAmount"] = purchase->getDiscountAmount();
    obj["finalPrice"] = purchase->getFinalPrice();
    obj["status"] = purchase->getStatusString();
    obj["purchasedAt"] = purchase->getPurchasedAt().toString(Qt::ISODate);
    return obj;
}

QJsonObject ClientHandler::reviewToJson(Review* review)
{
    QJsonObject obj;
    obj["reviewId"] = review->getReviewId();
    obj["userId"] = review->getUserId();
    obj["bookId"] = review->getBookId();
    obj["text"] = review->getText();
    obj["rating"] = review->getRating();
    obj["createdAt"] = review->getCreatedAt().toString(Qt::ISODate);
    obj["updatedAt"] = review->getUpdatedAt().toString(Qt::ISODate);
    return obj;
}