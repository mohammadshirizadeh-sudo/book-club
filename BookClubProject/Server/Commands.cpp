// Commands.cpp
#include "Commands.h"
#include "../Services/AuthService.h"
#include "../Services/UserService.h"
#include "../Services/BookService.h"
#include "../Services/CartService.h"
#include "../Services/PurchaseService.h"
#include "../Services/ReviewService.h"
#include "../Services/PublisherService.h"
#include "../Repositories/UserRepository.h"
#include "ClientHandler.h"
#include <QDebug>


// =============================================
// ===== Auth Commands =====
// =============================================


LoginCommand::LoginCommand(AuthService* authService, ClientHandler* clientHandler)
    : m_authService(authService)
    , m_clientHandler(clientHandler)
{
}

Response LoginCommand::execute(const QVariantMap& params)
{
    QString username = params["username"].toString();
    QString password = params["password"].toString();


    ValidationResult result = m_authService->login(username, password);

    if(!result.isValid){
        return Response::error(result.errorMessage);

    }

    User* user = m_authService->getUserByUsername(username);
    if (!user) {
        return Response::error("User created but could not be retrieved");
    }

        QVariantMap data ;

        int userId = user->getId();
        data["userId"] = userId;

        data["username"] = user->getUsername();
        QString role = user->getRoleString();
        data["role"] = role;
        m_clientHandler->setSession(userId, UserRepository::stringToRole(role));

        return Response::success("Login successful", data);

}

// ----- RegisterCommand -----
RegisterCommand::RegisterCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response RegisterCommand::execute(const QVariantMap& params)
{
    QString username = params["username"].toString();
    QString email = params["email"].toString();
    QString password = params["password"].toString();
    QString roleStr = params.value("role", "User").toString();


    UserRole role = UserRole::User;
    if (roleStr == "Publisher") role = UserRole::Publisher;
    else if (roleStr == "Admin") role = UserRole::Admin;



    ValidationResult result = m_authService->registerUser(username, email, password, role);

    if (!result.isValid) {
        return Response::error(result.errorMessage);
    }
    User* user = m_authService->getUserByUsername(username);


    if (!user) {
        return Response::error("User created but could not be retrieved");
    }

    QVariantMap data;
    data["userId"] = user->getId();
    data["username"] = user->getUsername();
    return Response::success("Registration successful", data);
}

// ----- LogoutCommand -----
LogoutCommand::LogoutCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response LogoutCommand::execute(const QVariantMap& params)
{
    if (m_authService->logout()) {
        return Response::success("Logout successful");
    }
    return Response::error("No user logged in");
}

// ----- ResetPasswordCommand -----
ResetPasswordCommand::ResetPasswordCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response ResetPasswordCommand::execute(const QVariantMap& params)
{
    QString email = params["email"].toString();

    if (m_authService->requestPasswordReset(email)) {
        return Response::success("Password reset link sent to your email");
    }
    return Response::error("Email not found");
}


// ----- ConfirmResetPasswordCommand -----
ConfirmResetPasswordCommand::ConfirmResetPasswordCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response ConfirmResetPasswordCommand::execute(const QVariantMap& params)
{
    QString token = params["token"].toString();
    QString newPassword = params["newPassword"].toString();

    if (m_authService->resetPasswordWithToken(token, newPassword))
        return Response::success("Password reset successfully");

    return Response::error("Invalid or expired token");
}

// =============================================
// ===== User Commands =====
// =============================================

// ----- GetProfileCommand -----
GetProfileCommand::GetProfileCommand(UserService* userService)
    : m_userService(userService)
{
}

Response GetProfileCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    User* user = m_userService->getProfile(userId);

    if (user) {
        QVariantMap data;
        data["id"] = user->getId();
        data["username"] = user->getUsername();
        data["email"] = user->getEmail();
        data["fullName"] = user->getFullname();
        data["role"] = user->getRoleString();
        data["status"] = static_cast<int>(user->getStatus());
        QStringList genreStrings;
        for (const Genre& genre : user->getFavouriteGenre()) {
            genreStrings.append(GenreHelper::toString(genre));
        }
        data["favoriteGenres"] = genreStrings;

        return Response::success(data);
    }
    return Response::error("User not found");
}

// ----- UpdateProfileCommand -----
UpdateProfileCommand::UpdateProfileCommand(UserService* userService)
    : m_userService(userService)
{
}

Response UpdateProfileCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QString email = params["email"].toString();
    QString fullName = params["fullName"].toString();
    QStringList genres = params["favoriteGenres"].toStringList();
    QVector<Genre> genreVector = GenreHelper::stringListToGenres(genres.toVector());

    if (m_userService->updateProfile(userId, email, fullName, genreVector)) {
        return Response::success("Profile updated successfully");
    }
    return Response::error("Failed to update profile");
}

// ----- ChangePasswordCommand -----
ChangePasswordCommand::ChangePasswordCommand(UserService* userService)
    : m_userService(userService)
{
}

Response ChangePasswordCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QString oldPassword = params["oldPassword"].toString();
    QString newPassword = params["newPassword"].toString();

    if (m_userService->changePassword(userId, oldPassword, newPassword)) {
        return Response::success("Password changed successfully");
    }
    return Response::error("Failed to change password");
}

// ----- UpdateFavoriteGenresCommand -----
UpdateFavoriteGenresCommand::UpdateFavoriteGenresCommand(UserService* userService)
    : m_userService(userService)
{
}

Response UpdateFavoriteGenresCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QStringList genres = params["genres"].toStringList();
    QVector<Genre> genreVector = GenreHelper::stringListToGenres(genres.toVector());

    if (m_userService->updateFavoriteGenres(userId, genreVector)) {
        return Response::success("Favorite genres updated");
    }
    return Response::error("Failed to update favorite genres");
}

// =============================================
// ===== Book Commands =====
// =============================================

// ----- SearchBooksCommand -----
SearchBooksCommand::SearchBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response SearchBooksCommand::execute(const QVariantMap& params)
{
    QString keyword = params["keyword"].toString();
    QVector<Book*> books = m_bookService->searchBooks(keyword);

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["price"] = book->getPrice();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetBookByIdCommand -----
GetBookByIdCommand::GetBookByIdCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBookByIdCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    Book* book = m_bookService->getBookById(bookId);

    if (book) {
        QVariantMap data;
        data["bookId"] = book->getBookId();
        data["title"] = book->getTitle();
        data["author"] = book->getAuthor();
        data["genre"] = GenreHelper::toString(book->getGenre());
        data["description"] = book->getDescription();
        data["price"] = book->getPrice();
        data["discountPercent"] = book->getDiscountPercent();
        data["finalPrice"] = book->getFinalPrice();
        data["averageRating"] = book->getAverageRating();
        data["salesCount"] = book->getSalesCount();
        data["isActive"] = book->getIsActive();
        data["coverPath"] = book->getCoverPath();
        data["pdfPath"] = book->getPdfPath();
        return Response::success(data);
    }
    return Response::error("Book not found");
}

// ----- GetBooksByGenreCommand -----
GetBooksByGenreCommand::GetBooksByGenreCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBooksByGenreCommand::execute(const QVariantMap& params)
{
    QString genre = params["genre"].toString();
    QVector<Book*> books = m_bookService->getBooksByGenre(genre);

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetPopularBooksCommand -----
GetPopularBooksCommand::GetPopularBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetPopularBooksCommand::execute(const QVariantMap& params)
{

    int limit = params.value("limit", 10).toInt();
    QVector<Book*> books = m_bookService->getPopularBooks(limit);

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetNewBooksCommand -----
GetNewBooksCommand::GetNewBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetNewBooksCommand::execute(const QVariantMap& params)
{
    int limit = params.value("limit", 10).toInt();
    QVector<Book*> books = m_bookService->getNewBooks(limit);

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetFreeBooksCommand -----
GetFreeBooksCommand::GetFreeBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetFreeBooksCommand::execute(const QVariantMap& params)
{
    QVector<Book*> books = m_bookService->getFreeBooks();

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetRecommendedBooksCommand -----
GetRecommendedBooksCommand::GetRecommendedBooksCommand(BookService* bookService ,UserService* m_userService)
    : m_bookService(bookService) , m_userService(m_userService)
{
}

Response GetRecommendedBooksCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    int limit = params.value("limit", 10).toInt();

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
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// =============================================
// ===== Cart Commands =====
// =============================================

// ----- AddToCartCommand -----
AddToCartCommand::AddToCartCommand(CartService* cartService)
    : m_cartService(cartService)
{
}

Response AddToCartCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    int bookId = params["bookId"].toInt();
    int quantity = params.value("quantity", 1).toInt();

    if (m_cartService->addToCart(userId, bookId, quantity)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Added to cart", data);
    }
    return Response::error("Failed to add to cart");
}

// ----- RemoveFromCartCommand -----
RemoveFromCartCommand::RemoveFromCartCommand(CartService* cartService)
    : m_cartService(cartService)
{
}

Response RemoveFromCartCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    int bookId = params["bookId"].toInt();

    if (m_cartService->removeFromCart(userId, bookId)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Removed from cart", data);
    }
    return Response::error("Failed to remove from cart");
}

// ----- UpdateCartQuantityCommand -----
UpdateCartQuantityCommand::UpdateCartQuantityCommand(CartService* cartService)
    : m_cartService(cartService)
{
}

Response UpdateCartQuantityCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    int bookId = params["bookId"].toInt();
    int quantity = params["quantity"].toInt();

    if (m_cartService->updateQuantity(userId, bookId, quantity)) {
        QVariantMap data;
        data["totalItems"] = m_cartService->getTotalItemCount(userId);
        data["finalPrice"] = m_cartService->getFinalPrice(userId);
        return Response::success("Cart updated", data);
    }
    return Response::error("Failed to update cart");
}

// ----- GetCartCommand -----
GetCartCommand::GetCartCommand(CartService* cartService)
    : m_cartService(cartService)
{
}

Response GetCartCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();

    QVariantMap data;
    QVector<CartItem> items = m_cartService->getCartItems(userId);

    QVariantList itemList;
    for (const CartItem& item : items) {
        QVariantMap itemData;
        itemData["bookId"] = item.getBookId();
        itemData["quantity"] = item.getQuantity();
        itemData["unitPrice"] = item.getUnitPrice();
        itemData["discountedPrice"] = item.getDiscountedPrice();
        itemData["totalPrice"] = item.getTotalPrice();
        itemData["totalDiscountedPrice"] = item.getTotalDiscountedPrice();
        itemList.append(itemData);
    }

    data["items"] = itemList;
    data["totalItems"] = m_cartService->getTotalItemCount(userId);
    data["totalPrice"] = m_cartService->getTotalPrice(userId);
    data["totalDiscount"] = m_cartService->getTotalDiscount(userId);
    data["finalPrice"] = m_cartService->getFinalPrice(userId);
    data["isEmpty"] = m_cartService->isEmpty(userId);

    return Response::success(data);
}

// ----- ClearCartCommand -----
ClearCartCommand::ClearCartCommand(CartService* cartService)
    : m_cartService(cartService)
{
}

Response ClearCartCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    m_cartService->clearCart(userId);
    return Response::success("Cart cleared");
}

// =============================================
// ===== Purchase Commands =====
// =============================================

// ----- CheckoutCommand -----
CheckoutCommand::CheckoutCommand(PurchaseService* purchaseService)
    : m_purchaseService(purchaseService)
{
}

Response CheckoutCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();

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

// ----- GetPurchaseHistoryCommand -----
GetPurchaseHistoryCommand::GetPurchaseHistoryCommand(PurchaseService* purchaseService)
    : m_purchaseService(purchaseService)
{
}

Response GetPurchaseHistoryCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QVector<Purchase*> purchases = m_purchaseService->getPurchaseHistory(userId);

    QVariantList purchaseList;
    for (Purchase* purchase : purchases) {
        QVariantMap purchaseData;
        purchaseData["purchaseId"] = purchase->getPurchaseId();
        purchaseData["finalPrice"] = purchase->getFinalPrice();
        purchaseData["totalItems"] = purchase->getTotalItemCount();
        purchaseData["status"] = purchase->getStatusString();
        purchaseData["purchasedAt"] = purchase->getPurchasedAt().toString(Qt::ISODate);
        purchaseList.append(purchaseData);
    }

    QVariantMap data;
    data["purchases"] = purchaseList;
    data["count"] = purchaseList.size();
    return Response::success(data);
}

// ----- GetPurchaseByIdCommand -----
GetPurchaseByIdCommand::GetPurchaseByIdCommand(PurchaseService* purchaseService)
    : m_purchaseService(purchaseService)
{
}

Response GetPurchaseByIdCommand::execute(const QVariantMap& params)
{
    int purchaseId = params["purchaseId"].toInt();
    Purchase* purchase = m_purchaseService->getPurchaseById(purchaseId);

    if (purchase) {
        QVariantMap data;
        data["purchaseId"] = purchase->getPurchaseId();
        data["userId"] = purchase->getUserId();
        data["totalPrice"] = purchase->getTotalPrice();
        data["discountAmount"] = purchase->getDiscountAmount();
        data["finalPrice"] = purchase->getFinalPrice();
        data["status"] = purchase->getStatusString();
        data["purchasedAt"] = purchase->getPurchasedAt().toString(Qt::ISODate);
        return Response::success(data);
    }
    return Response::error("Purchase not found");
}

// =============================================
// ===== Review Commands =====
// =============================================

// ----- AddReviewCommand -----
AddReviewCommand::AddReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response AddReviewCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    int bookId = params["bookId"].toInt();
    QString text = params["text"].toString();
    int rating = params["rating"].toInt();

    if (m_reviewService->addReview(userId, bookId, text, rating)) {
        QVariantMap data;
        data["bookId"] = bookId;
        data["rating"] = rating;
        data["averageRating"] = m_reviewService->getAverageRating(bookId);
        return Response::success("Review added", data);
    }
    return Response::error("Failed to add review");
}

// ----- EditReviewCommand -----
EditReviewCommand::EditReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response EditReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params["reviewId"].toInt();
    int userId = params["userId"].toInt();
    QString text = params["text"].toString();
    int rating = params["rating"].toInt();

    if (m_reviewService->editReview(reviewId, userId, text, rating)) {
        return Response::success("Review updated");
    }
    return Response::error("Failed to update review");
}

// ----- DeleteReviewCommand -----
DeleteReviewCommand::DeleteReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response DeleteReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params["reviewId"].toInt();
    int userId = params["userId"].toInt();

    if (m_reviewService->deleteReview(reviewId, userId)) {
        return Response::success("Review deleted");
    }
    return Response::error("Failed to delete review");
}

// ----- GetReviewsForBookCommand -----
GetReviewsForBookCommand::GetReviewsForBookCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response GetReviewsForBookCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    QVector<Review*> reviews = m_reviewService->getReviewsForBook(bookId);

    QVariantList reviewList;
    for (Review* review : reviews) {
        QVariantMap reviewData;
        reviewData["reviewId"] = review->getReviewId();
        reviewData["userId"] = review->getUserId();
        reviewData["text"] = review->getText();
        reviewData["rating"] = review->getRating();
        reviewData["createdAt"] = review->getCreatedAt().toString(Qt::ISODate);
        reviewList.append(reviewData);
    }

    QVariantMap data;
    data["reviews"] = reviewList;
    data["count"] = reviewList.size();
    data["averageRating"] = m_reviewService->getAverageRating(bookId);
    return Response::success(data);
}

// ----- GetAverageRatingCommand -----
GetAverageRatingCommand::GetAverageRatingCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response GetAverageRatingCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    double avg = m_reviewService->getAverageRating(bookId);

    QVariantMap data;
    data["bookId"] = bookId;
    data["averageRating"] = avg;
    return Response::success(data);
}

// =============================================
// ===== Publisher Commands =====
// =============================================

// ----- AddBookCommand -----
AddBookCommand::AddBookCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response AddBookCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QString title = params["title"].toString();
    QString author = params["author"].toString();
    Genre genre = GenreHelper::fromString(params["genre"].toString());
    QString description = params["description"].toString();
    double price = params["price"].toDouble();
    double discountPercent = params.value("discountPercent", 0.0).toDouble();



    if (m_publisherService->addBook(publisherId, title, author, genre, description, price)) {
        QVariantMap data;
        data["title"] = title;
        data["author"] = author;
        return Response::success("Book added successfully", data);
    }
    return Response::error("Failed to add book");
}

// ----- EditBookCommand -----
EditBookCommand::EditBookCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response EditBookCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    QString title = params["title"].toString();
    QString author = params["author"].toString();
    Genre genre = GenreHelper::fromString(params["genre"].toString());
    QString description = params["description"].toString();
    double price = params["price"].toDouble();
    double discount = params["discount"].toDouble();

    if (m_publisherService->getBookService()->editBook( bookId, title, author, genre, description, price, discount)) {

        return Response::success("Book updated successfully");
    }
    return Response::error("Failed to update book");
}

// ----- DeactivateBookCommand -----
DeactivateBookCommand::DeactivateBookCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response DeactivateBookCommand::execute(const QVariantMap& params)
{

    int bookId = params["bookId"].toInt();

    if (m_publisherService->getBookService()->deactivateBook(bookId)) {
        return Response::success("Book deactivated");
    }
    return Response::error("Failed to deactivate book");
}

// ----- ReactivateBookCommand -----
ReactivateBookCommand::ReactivateBookCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response ReactivateBookCommand::execute(const QVariantMap& params)
{

    int bookId = params["bookId"].toInt();

    if (m_publisherService->getBookService()->reactivateBook(bookId)) {
        return Response::success("Book reactivated");
    }
    return Response::error("Failed to reactivate book");
}

// ----- GetPublisherBooksCommand -----
GetPublisherBooksCommand::GetPublisherBooksCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response GetPublisherBooksCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QVector<Book*> books = m_publisherService->getBooksByPublisher(publisherId);

    QVariantList bookList;
    for (Book* book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(data);
}

// ----- GetPublisherStatsCommand -----
GetPublisherStatsCommand::GetPublisherStatsCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response GetPublisherStatsCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QMap<QString, QVariant> stats = m_publisherService->getSalesStatistics(publisherId);


    QVariantMap data;
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        data[it.key()] = it.value();
    }
    return Response::success(data);
}

// =============================================
// ===== Admin Commands =====
// =============================================

// ----- BlockUserCommand -----
BlockUserCommand::BlockUserCommand(UserService* adminService)
    : m_adminService(adminService)
{
}

Response BlockUserCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QString reason = params["reason"].toString();

    if (m_adminService->blockUser(userId, reason)) {
        return Response::success("User blocked");
    }
    return Response::error("Failed to block user");
}

// ----- UnblockUserCommand -----
UnblockUserCommand::UnblockUserCommand(UserService* adminService)
    : m_adminService(adminService)
{
}

Response UnblockUserCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();

    if (m_adminService->unblockUser(userId)) {
        return Response::success("User unblocked");
    }
    return Response::error("Failed to unblock user");
}

// ----- DeleteUserCommand -----
DeleteUserCommand::DeleteUserCommand(UserService* adminService)
    : m_adminService(adminService)
{
}


Response DeleteUserCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();

    if (m_adminService->deleteUser(userId)) {
        return Response::success("User deleted");
    }
    return Response::error("Failed to delete user");
}

// ----- GetAllUsersCommand -----
GetAllUsersCommand::GetAllUsersCommand(UserService* adminService)
    : m_adminService(adminService)
{
}

Response GetAllUsersCommand::execute(const QVariantMap& params)
{
    QVector<User*> users = m_adminService->getAllUsers();

    QVariantList userList;
    for (User* user : users) {
        QVariantMap userData;
        userData["id"] = user->getId();
        userData["username"] = user->getUsername();
        userData["email"] = user->getEmail();
        userData["fullName"] = user->getFullname();
        userData["role"] = user->getRoleString();
        userData["status"] = static_cast<int>(user->getStatus());
        userList.append(userData);
    }

    QVariantMap data;
    data["users"] = userList;
    data["count"] = userList.size();
    return Response::success(data);
}

// ----- GetBlockedUsersCommand -----
GetBlockedUsersCommand::GetBlockedUsersCommand(UserService* adminService)
    : m_adminService(adminService)
{
}

Response GetBlockedUsersCommand::execute(const QVariantMap& params)
{
    QVector<User*> users = m_adminService->getBlockedUsers();

    QVariantList userList;
    for (User* user : users) {
        QVariantMap userData;
        userData["id"] = user->getId();
        userData["username"] = user->getUsername();
        userData["email"] = user->getEmail();
        userData["fullName"] = user->getFullname();
        userData["role"] = user->getRoleString();
        userList.append(userData);
    }

    QVariantMap data;
    data["users"] = userList;
    data["count"] = userList.size();
    return Response::success(data);
}

// ----- AdminDeleteBookCommand -----
AdminDeleteBookCommand::AdminDeleteBookCommand(UserService* adminService , BookService* bookService)
    : m_adminService(adminService) , m_bookService(bookService)
{
}

Response AdminDeleteBookCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    QString reason = params["reason"].toString();

    if (m_bookService->deleteBook(bookId)) {

        return Response::success("Book deleted by admin");
    }
    return Response::error("Failed to delete book");
}

// ----- AdminDeleteReviewCommand -----
AdminDeleteReviewCommand::AdminDeleteReviewCommand(UserService* adminService , ReviewService* reviewService)
    : m_adminService(adminService) , m_reviewService(reviewService)
{
}

Response AdminDeleteReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params["reviewId"].toInt();
    int userId = params["userId"].toInt();
    QString reason = params["reason"].toString();

    if (m_reviewService->deleteReview(reviewId ,userId)) {
        return Response::success("Review deleted by admin");
    }
    return Response::error("Failed to delete review");
}

// ----- GetSystemStatsCommand -----
GetSystemStatsCommand::GetSystemStatsCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response GetSystemStatsCommand::execute(const QVariantMap& params)
{
    QMap<QString, QVariant> stats = m_adminService->getSystemStats();


    QVariantMap data;
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        data[it.key()] = it.value();
    }
    return Response::success(data);
}