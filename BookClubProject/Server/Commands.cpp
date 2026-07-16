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
#include "Request.h"
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
    qDebug() << "[COMMAND EXECUTE] Login command started for user:" << params["username"].toString();
    QString username = params["username"].toString();
    QString password = params["password"].toString();


    ValidationResult result = m_authService->login(username, password);


    if(!result.isValid){
        return Response::error(CommandType::Login,result.errorMessage);
    }

    User* user = m_authService->getUserByUsername(username);
    if (!user) {
        return Response::error(CommandType::Login,"User created but could not be retrieved");
    }

        QVariantMap data ;

        int userId = user->getId();
        data["userId"] = userId;

        data["username"] = user->getUsername();
        QString role = user->getRoleString();
        data["role"] = role;
        m_clientHandler->setSession(userId, UserRepository::stringToRole(role));

        return Response::success(CommandType::Login,"Login successful", data);

}

// ----- RegisterCommand -----
RegisterCommand::RegisterCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response RegisterCommand::execute(const QVariantMap& params)
{
    QString fullname = params["fullName"].toString();
    QString username = params["username"].toString();
    QString email = params["email"].toString();
    QString password = params["password"].toString();
    QString roleStr = params["role"].toString();



    UserRole role = UserRole::User;
    if (roleStr == "Publisher") role = UserRole::Publisher;
    else if (roleStr == "Admin") role = UserRole::Admin;



    ValidationResult result = m_authService->registerUser(fullname , username, email, password, role);

    if (!result.isValid) {
        return Response::error(CommandType::Register,result.errorMessage);
    }
    User* user = m_authService->getUserByUsername(username);


    if (!user) {
        return Response::error(CommandType::Register , "User created but could not be retrieved");
    }

    QVariantMap data;
    data["userId"] = user->getId();
    data["username"] = user->getUsername();
    data["role"] = roleStr;
    return Response::success(CommandType::Register , "Registration successful", data);
}

// ----- LogoutCommand -----
LogoutCommand::LogoutCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response LogoutCommand::execute(const QVariantMap& params)
{
    if (m_authService->logout()) {
        return Response::success(CommandType::Logout ,"Logout successful");
    }
    return Response::error(CommandType::Logout , "No user logged in");
}

// ----- ResetPasswordCommand -----
ResetPasswordCommand::ResetPasswordCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response ResetPasswordCommand::execute(const QVariantMap& params)
{
    QString email = params["email"].toString();

    if (m_authService->requestPasswordReset(email).isValid) {
        return Response::success(CommandType::ResetPassword , "Password reset link sent to your email");
    }
    return Response::error(CommandType::ResetPassword , "Email not found");
}



//check this !!!!!!!!!!!!!
// ----- ConfirmResetPasswordCommand -----
ConfirmResetPasswordCommand::ConfirmResetPasswordCommand(AuthService* authService)
    : m_authService(authService)
{
}

Response ConfirmResetPasswordCommand::execute(const QVariantMap& params)
{
    QString token = params["token"].toString();
    QString newPassword = params["newPassword"].toString();

    if (m_authService->resetPasswordWithToken(token, newPassword).isValid)
        return Response::success(CommandType::ResetPasswordWithToken , "Password reset successfully");

    return Response::error(CommandType::ResetPasswordWithToken , "Invalid or expired token");
}

// =============================================
// ===== User Commands =====
// =============================================

// ----- GetProfileCommand -----
GetProfileCommand::GetProfileCommand(UserService* userService,PurchaseService* m_purchaseService)
    : m_userService(userService), m_purchaseService(m_purchaseService)
{
}

Response GetProfileCommand::execute(const QVariantMap& params)
{
    qDebug()<<"Enter to getprofile execute";
    int userId = params["userId"].toInt();
    User* user = m_userService->getProfile(userId);
    AccountStatus status = user->getStatus();

    if (user) {
        QVariantMap data;
        data["id"] = user->getId();
        data["username"] = user->getUsername();
        data["email"] = user->getEmail();
        data["fullName"] = user->getFullname();
        data["role"] = user->getRoleString();
        data["status"] = m_userService->getStringStatus(status);
        QStringList genreStrings;
        for (const Genre& genre : user->getFavouriteGenre()) {
            genreStrings.append(GenreHelper::toString(genre));
        }
        data["favoriteGenres"] = genreStrings;


        if(user->getRole()==UserRole::User){
            qDebug()<<"Enter to if role ";
            int purchaseCount = m_purchaseService->getPurchaseCount(userId);

            data["purchaseCount"] = purchaseCount;
            qDebug()<<"Exit if role";
        }

        if(user->getRole() == UserRole::Publisher){
            Publisher* publisher = static_cast<Publisher*>(user);
            data["publisherName"] = publisher->getPublisherName();
            data["totalRevenue"] = publisher->getTotalRevenue();
            data["joinedAt"] = publisher->getJoinedAt();

        }

        return Response::success(CommandType::GetProfile , data);
    }
    return Response::error(CommandType::GetProfile , "User not found");
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
    QString userName = params["userName"].toString();
    ValidationResult result = m_userService->updateProfile(userId, email, fullName, userName);

    if (result.isValid) {
        return Response::success(CommandType::UpdateProfile , "Profile updated successfully");
    }
    return Response::error(CommandType::UpdateProfile ,result.errorMessage);
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
        return Response::success(CommandType::ChangePassword, "Password changed successfully");
    }
    return Response::error(CommandType::ChangePassword ,"Failed to change password");
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
        return Response::success(CommandType::UpdateFavoriteGenres , "Favorite genres updated");
    }
    return Response::error(CommandType::UpdateFavoriteGenres ,"Failed to update favorite genres");
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
    QVector<QSharedPointer<Book>> books = m_bookService->searchBooks(keyword);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::SearchBooks, data);
}

// ----- GetBookByIdCommand -----
GetBookByIdCommand::GetBookByIdCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBookByIdCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    QSharedPointer<Book> book = m_bookService->getBookById(bookId);

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
        return Response::success(CommandType::GetBookById , data);
    }
    return Response::error(CommandType::GetBookById ,"Book not found");
}

// ----- GetBooksByGenreCommand -----
GetBooksByGenreCommand::GetBooksByGenreCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBooksByGenreCommand::execute(const QVariantMap& params)
{
    QString genre = params["genre"].toString();
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByGenre(genre);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::GetBooksByGenre , data);
}

// ----- GetPopularBooksCommand -----
GetPopularBooksCommand::GetPopularBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetPopularBooksCommand::execute(const QVariantMap& params)
{

    int limit = params.value("limit", 10).toInt();
    QVector<QSharedPointer<Book>> books = m_bookService->getPopularBooks(limit);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::GetPopularBooks , data);
}

// ----- GetNewBooksCommand -----
GetNewBooksCommand::GetNewBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetNewBooksCommand::execute(const QVariantMap& params)
{
    int limit = params.value("limit", 10).toInt();
    QVector<QSharedPointer<Book>> books = m_bookService->getNewBooks(limit);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::GetNewBooks , data);
}

// ----- GetFreeBooksCommand -----
GetFreeBooksCommand::GetFreeBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetFreeBooksCommand::execute(const QVariantMap& params)
{

    qDebug() << "🔍 [Server DB] Executing Free Books SQL Query...";
    QVector<QSharedPointer<Book>> books = m_bookService->getFreeBooks();

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());//this
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        bookList.append(bookData);
    }
    qDebug() << "📦 [Server DB] Successfully pulled"
             << bookList.size()
             << "books from database.";

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetFreeBooks , data);
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
        return Response::error(CommandType::GetRecommendedBooks ,"User not found");
    }

    QVector<QSharedPointer<Book>> books = m_bookService->getRecommendedBooks(
        user->getFavouriteGenre(),
        limit
        );

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::GetRecommendedBooks , data);
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
        return Response::success(CommandType::AddToCart ,"Added to cart", data);
    }
    return Response::error(CommandType::AddToCart,"Failed to add to cart");
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
        return Response::success(CommandType::RemoveFromCart , "Removed from cart", data);
    }
    return Response::error(CommandType::RemoveFromCart ,"Failed to remove from cart");
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
        return Response::success(CommandType::UpdateCartQuantity , "Cart updated", data);
    }
    return Response::error(CommandType::UpdateCartQuantity ,"Failed to update cart");
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

    return Response::success(CommandType::GetCart , data);
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
    return Response::success(CommandType::ClearCart , "Cart cleared");
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

    QSharedPointer<Purchase> purchase = m_purchaseService->checkout(userId);
    if (purchase) {
        QVariantMap data;
        data["purchaseId"] = purchase->getPurchaseId();
        data["finalPrice"] = purchase->getFinalPrice();
        data["totalItems"] = purchase->getTotalItemCount();
        data["status"] = purchase->getStatusString();
        return Response::success(CommandType::Checkout, "Purchase successful", data);
    }
    return Response::error(CommandType::Checkout ,"Purchase failed");
}

// ----- GetPurchaseHistoryCommand -----
GetPurchaseHistoryCommand::GetPurchaseHistoryCommand(PurchaseService* purchaseService)
    : m_purchaseService(purchaseService)
{
}

Response GetPurchaseHistoryCommand::execute(const QVariantMap& params)
{
    int userId = params["userId"].toInt();
    QVector<QSharedPointer<Purchase>> purchases = m_purchaseService->getPurchaseHistory(userId);

    QVariantList purchaseList;
    for (QSharedPointer<Purchase> purchase : purchases) {
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
    return Response::success(CommandType::GetPurchaseHistory , data);
}

// ----- GetPurchaseByIdCommand -----
GetPurchaseByIdCommand::GetPurchaseByIdCommand(PurchaseService* purchaseService)
    : m_purchaseService(purchaseService)
{
}

Response GetPurchaseByIdCommand::execute(const QVariantMap& params)
{
    int purchaseId = params["purchaseId"].toInt();
    QSharedPointer<Purchase> purchase = m_purchaseService->getPurchaseById(purchaseId);

    if (purchase) {
        QVariantMap data;
        data["purchaseId"] = purchase->getPurchaseId();
        data["userId"] = purchase->getUserId();
        data["totalPrice"] = purchase->getTotalPrice();
        data["discountAmount"] = purchase->getDiscountAmount();
        data["finalPrice"] = purchase->getFinalPrice();
        data["status"] = purchase->getStatusString();
        data["purchasedAt"] = purchase->getPurchasedAt().toString(Qt::ISODate);
        return Response::success(CommandType::GetPurchaseById,  data);
    }
    return Response::error(CommandType::GetPurchaseById ,"Purchase not found");
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
        return Response::success(CommandType::AddReview ,"Review added", data);
    }
    return Response::error(CommandType::AddReview ,"Failed to add review");
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
        return Response::success(CommandType::EditReview, "Review updated");
    }
    return Response::error(CommandType::EditBook ,"Failed to update review");
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
        return Response::success(CommandType::DeleteReview, "Review deleted");
    }


    //you should fixe deleteownreview and review commands !!!!!!!!!!!
    return Response::error(CommandType::DeleteReview ,"Failed to delete review");
}


/*
class AdminDeleteReviewCommand : public Command
{
public:
    explicit AdminDeleteReviewCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteReview; }
    QString getName() const override { return "AdminDeleteReview"; }
    bool requiresAdmin() const override { return true; }

private:
    ReviewService* m_reviewService;
};

*/

// ----- GetReviewsForBookCommand -----
GetReviewsForBookCommand::GetReviewsForBookCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response GetReviewsForBookCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    QVector<QSharedPointer<Review>> reviews = m_reviewService->getReviewsForBook(bookId);

    QVariantList reviewList;
    for (QSharedPointer<Review> review : reviews) {
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
    return Response::success(CommandType::GetReviewsForBook , data);
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
    return Response::success(CommandType::GetAverageRating , data);
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
        return Response::success(CommandType::AddBook , "Book added successfully", data);
    }
    return Response::error(CommandType::AddBook ,"Failed to add book");
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

        return Response::success(CommandType::EditBook , "Book updated successfully");
    }
    return Response::error(CommandType::EditBook ,"Failed to update book");
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
        return Response::success(CommandType::DeactivateBook , "Book deactivated");
    }
    return Response::error(CommandType::DeactivateBook ,"Failed to deactivate book");
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
        return Response::success(CommandType::ReactivateBook ,"Book reactivated");
    }
    return Response::error(CommandType::ReactivateBook ,"Failed to reactivate book");
}

// ----- GetPublisherBooksCommand -----
GetPublisherBooksCommand::GetPublisherBooksCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response GetPublisherBooksCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QVector<QSharedPointer<Book>> books = m_publisherService->getBooksByPublisher(publisherId);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
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
    return Response::success(CommandType::GetPublisherBooks, data);
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
    return Response::success(CommandType::GetPublisherStats, data);
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
        return Response::success(CommandType::BlockUser ,"User blocked");
    }
    return Response::error(CommandType::BlockUser ,"Failed to block user");
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
        return Response::success(CommandType::UnblockUser , "User unblocked");
    }
    return Response::error(CommandType::UnblockUser , "Failed to unblock user");
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
        return Response::success(CommandType::DeleteUser , "User deleted");
    }
    return Response::error(CommandType::DeleteUser , "Failed to delete user");
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
    return Response::success(CommandType::GetAllUsers , data);
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
    return Response::success(CommandType::GetBlockedUsers, data);
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

        return Response::success(CommandType::DeleteBook , "Book deleted by admin");
    }

    //you should fix deleteownbook and book!!!!!!!!!!!!!!
    return Response::error(CommandType::DeleteBook , "Failed to delete book");
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
        return Response::success(CommandType::DeleteReview ,"Review deleted by admin");
    }


    //you should fix deleteownreview and review
    return Response::error(CommandType::DeleteReview , "Failed to delete review");
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
    return Response::success(CommandType::GetSystemStats , data);
}
RequestPasswordResetCommand::RequestPasswordResetCommand(AuthService *authService)
    :m_authService(authService)
{

}


/*
Response RequestPasswordResetCommand::execute(const QVariantMap& params)
{
    QString email = params.value("email").toString();

    if (email.isEmpty()) {
        return Response::error(CommandType::RequestPasswordReset ,"Email address is required");
    }
    ValidationResult result = m_authService->requestPasswordReset(email);

    if (result.isValid) {
        return Response::success(CommandType::RequestPasswordReset, "Password reset link sent to your email");
    } else {
        return Response::error(CommandType::RequestPasswordReset ,result.errorMessage);
    }
}
*/


Response RequestPasswordResetCommand::execute(const QVariantMap& params)
{
    QString email = params.value("email").toString();

    if (email.isEmpty()) {
        return Response::error(CommandType::RequestPasswordReset, "Email address is required");
    }

    // 1. درخواست ریست پسورد (توکن تولید می‌شود)
    ValidationResult result = m_authService->requestPasswordReset(email);
    User* user = m_authService->getUserByEmail(email);
    QString token = user->getPasswordResetToken();
    QDateTime expiry = user->getResetTokenExpiry();

    if (!result.isValid) {
        return Response::error(CommandType::RequestPasswordReset, result.errorMessage);
    }

    // 3. برگرداندن توکن به کلاینت
    QVariantMap responseData;
    responseData["token"] = token;
    responseData["email"] = email;
    responseData["expiry"] = expiry.toString(Qt::ISODate);

    return Response::success(
        CommandType::RequestPasswordReset,
        "Password reset link sent to your email",
        responseData
        );
}

ResetPasswordWithTokenCommand::ResetPasswordWithTokenCommand(AuthService *authService)
    :m_authService(authService)
{

}

Response ResetPasswordWithTokenCommand::execute(const QVariantMap& params)
{
    QString token = params.value("token").toString();
    QString newPassword = params.value("newPassword").toString();

    if (token.isEmpty()) {
        return Response::error(CommandType::ResetPasswordWithToken, "Reset token is required");
    }
    if (newPassword.isEmpty()) {
        return Response::error(CommandType::ResetPasswordWithToken , "New password is required");
    }

    ValidationResult result = m_authService->resetPasswordWithToken(token, newPassword);
    if (result.isValid) {
        QVariantMap data = result.getData();

        return Response::success(CommandType::ResetPasswordWithToken, "Password reset successfully", data);
    } else {
        return Response::error(CommandType::ResetPasswordWithToken , result.errorMessage);
    }
}



// Commands.cpp
SearchUserCommand::SearchUserCommand(UserService* userService , BookService* bookService)
    : m_userService(userService), m_bookService(bookService)
{
}

Response SearchUserCommand::execute(const QVariantMap& params)
{
    // 1. دریافت کلمه جستجو
    QString keyword = params.value("keyword").toString();

    if (keyword.isEmpty()) {
        return Response::error(CommandType::SearchUsers, "Keyword is required");
    }

    QVector<User*> users = m_userService->searchUsers(keyword);

    if (users.isEmpty()) {
        return Response::error(CommandType::SearchUsers, "No users found");
    }
    QVariantList userList;
    for (User* user : users) {
        QVariantMap userData;
        userData["id"] = user->getId();
        userData["username"] = user->getUsername();
        userData["email"] = user->getEmail();
        userData["role"] = user->getRoleString();
        if (user->isPublisher()) {
            Publisher* publisher = static_cast<Publisher*>(user);

            userData["publisherName"] = publisher->getPublisherName();
            userData["totalRevenue"] = publisher->getTotalRevenue();
            userData["joinedAt"] = publisher->getJoinedAt().toString(Qt::ISODate);
            QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(user->getId());
            userData["publishedBooksCount"] = books.size();

            QVariantList bookList;
            for (QSharedPointer<Book> book : books) {
                QVariantMap bookData;
                bookData["bookId"] = book->getBookId();
                bookData["title"] = book->getTitle();
                bookData["author"]          = book->getAuthor();
                bookData["genre"]           = GenreHelper::toString(book->getGenre());
                bookData["price"]           = book->getPrice();
                bookData["discountPercent"] = book->getDiscountPercent();
                bookData["finalPrice"]      = book->getFinalPrice();
                bookData["averageRating"]   = book->getAverageRating();
                bookData["coverPath"]       = book->getCoverPath();
                bookList.append(bookData);
            }
            userData["books"] = bookList;

        } else {
            // ✅ کاربر عادی: اطلاعات عمومی
            userData["fullName"] = user->getFullname();
            userData["status"] = m_userService->getStringStatus(user->getStatus());
            userData["favoriteGenres"] = QVariant::fromValue(user->getFavouriteGenre());
            userData["createdAt"] = user->getCreatedAt().toString(Qt::ISODate);
        }

        userList.append(userData);
    }

    QVariantMap data;
    data["users"] = userList;
    data["count"] = userList.size();

    return Response::success(CommandType::SearchUsers, "Search completed", data);
}

SearchAuthorCommand::SearchAuthorCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response SearchAuthorCommand::execute(const QVariantMap& params)
{
    // 1. دریافت کلمه جستجو
    QString keyword = params.value("keyword").toString();

    if (keyword.isEmpty()) {
        return Response::error(CommandType::SearchAuthors, "Keyword is required");
    }

    QMap<QString, QVector<QSharedPointer<Book>>> authorBooks = m_bookService->searchAuthorsWithBooks(keyword);

    if (authorBooks.isEmpty()) {
        return Response::error(CommandType::SearchAuthors, "No authors found");
    }

    QVariantList authorList;
    for (auto it = authorBooks.begin(); it != authorBooks.end(); ++it) {
        QString authorName = it.key();
        QVector<QSharedPointer<Book>> books = it.value();

        QVariantMap authorData;
        authorData["author"] = authorName;
        authorData["bookCount"] = books.size();
        QVariantList bookList;
        for (QSharedPointer<Book> book : books) {
            QVariantMap bookData;
            bookData["bookId"] = book->getBookId();
            bookData["title"] = book->getTitle();
            bookData["author"]          = book->getAuthor();
            bookData["genre"]           = GenreHelper::toString(book->getGenre());
            bookData["price"]           = book->getPrice();
            bookData["discountPercent"] = book->getDiscountPercent();
            bookData["finalPrice"]      = book->getFinalPrice();
            bookData["averageRating"]   = book->getAverageRating();
            bookData["coverPath"]       = book->getCoverPath();

            bookList.append(bookData);
        }
        authorData["books"] = bookList;

        authorList.append(authorData);
    }

    QVariantMap data;
    data["authors"] = authorList;
    data["count"] = authorList.size();

    return Response::success(CommandType::SearchAuthors, "Search completed", data);
}


