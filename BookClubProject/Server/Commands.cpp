// Commands.cpp
#include "Commands.h"
#include "../Services/AuthService.h"
#include "../Services/UserService.h"
#include "../Services/BookService.h"
#include "../Services/CartService.h"
#include "../Services/PurchaseService.h"
#include "../Services/ReviewService.h"
#include "../Services/PublisherService.h"
#include "../Services/LibraryService.h"
#include <QFileInfo>
#include <QDir>
#include <QUuid>

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

    m_clientHandler->setSession(userId, UserRepository::stringToRole(role), user->getUsername());

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
    if (!user) { return Response::error( CommandType::GetProfile, "User not found"); }
    AccountStatus status = user->getStatus();

    if (user) {
        QVariantMap data;
        data["id"] = user->getId();
        data["username"] = user->getUsername();
        data["email"] = user->getEmail();
        data["fullName"] = user->getFullname();
        data["role"] = user->getRoleString();
        data["status"] = m_userService->getStringStatus(status);
        data["updatedAt"] = user->getUpdatedAt();



        if(user->getRole()==UserRole::User){
            QStringList genreStrings;
            for (const Genre& genre : user->getFavouriteGenre()) {
                genreStrings.append(GenreHelper::toString(genre));
            }
            data["favoriteGenres"] = genreStrings;
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
SearchBooksCommand::SearchBooksCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)

{
}

Response SearchBooksCommand::execute(const QVariantMap& params)
{
    QString keyword = params["keyword"].toString();
    QString status = params["status"].toString();
    int userId = params["userId"].toInt();
    QVector<QSharedPointer<Book>> books;
    if(status == "publisher") {
        qDebug()<<"we are in command";
        books = m_bookService->searchBooksByPublisher(keyword);
        qDebug()<< "we are out of search publisher";
    }else if(status == "author"){
        books = m_bookService->searchBooksByAuthor(keyword);
    }else{
        books = m_bookService->searchBooks(keyword);
    }


    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = bookId;
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["price"] = book->getPrice();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
        }
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::SearchBooks, data);
}

// ----- GetBookByIdCommand -----
GetBookByIdCommand::GetBookByIdCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)
{
}

Response GetBookByIdCommand::execute(const QVariantMap& params)
{
    int bookId = params["bookId"].toInt();
    int userId = params["userId"].toInt();
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


        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            data["isFavorite"] = isFavorite;
        }
        return Response::success(CommandType::GetBookById , data);
    }
    return Response::error(CommandType::GetBookById ,"Book not found");
}

// ----- GetBooksByGenreCommand -----
GetBooksByGenreCommand::GetBooksByGenreCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService), m_userService(userService)
{
}

Response GetBooksByGenreCommand::execute(const QVariantMap& params)
{
    QString genre = params["genre"].toString();
    int userId = params["userId"].toInt();
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByGenre(genre);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["description"] = book->getDescription();
        bookData["price"] = book->getPrice();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        bookData["isActive"] = book->getIsActive();
        bookData["coverPath"] = book->getCoverPath();
        bookData["pdfPath"] = book->getPdfPath();
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
        }
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetBooksByGenre , data);
}


// Commands.cpp
GetAllGenresCommand::GetAllGenresCommand()
{
}
Response GetAllGenresCommand::execute(const QVariantMap& params)
{
    // دریافت لیست همه ژانرها از GenreHelper
    QVector<QString> genres = GenreHelper::getAllGenres();

    // تبدیل به QVariantList برای پاسخ
    QVariantList genreList;
    for (const QString& genre : genres) {
        genreList.append(genre);
    }

    QVariantMap data;
    data["genres"] = genreList;
    data["count"] = genreList.size();

    return Response::success(CommandType::GetAllGenres, "Genres loaded successfully", data);
}




// ----- GetPopularBooksCommand -----
GetPopularBooksCommand::GetPopularBooksCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)
{
}

Response GetPopularBooksCommand::execute(const QVariantMap& params)
{

    int limit = params.value("limit", 10).toInt();
    int userId = params["userId"].toInt();
    QVector<QSharedPointer<Book>> books = m_bookService->getPopularBooks(limit);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {

        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = bookId;
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
            qDebug()<<"the bool in command from book " << book->getTitle() << "is " << isFavorite;
        }

        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetPopularBooks , data);
}

// ----- GetNewBooksCommand -----
GetNewBooksCommand::GetNewBooksCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)
{
}

Response GetNewBooksCommand::execute(const QVariantMap& params)
{
    int limit = params.value("limit", 10).toInt();
    int userId = params["userId"].toInt();
    QVector<QSharedPointer<Book>> books = m_bookService->getNewBooks(limit);

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = bookId;
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
        }
        bookData["genre"] =GenreHelper::toString(book->getGenre());
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["pdfPath"] = book->getPdfPath();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetNewBooks , data);
}

// ----- GetFreeBooksCommand -----
GetFreeBooksCommand::GetFreeBooksCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)
{
}

Response GetFreeBooksCommand::execute(const QVariantMap& params)
{

    qDebug() << "🔍 [Server DB] Executing Free Books SQL Query...";
    QVector<QSharedPointer<Book>> books = m_bookService->getFreeBooks();

    int userId = params["userId"].toInt();

    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = bookId;
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());//this
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        bookData["pdfPath"] = book->getPdfPath();
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
        }
        bookData["discountPercent"] = book->getDiscountPercent();
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
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["pdfPath"] = book->getPdfPath();
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
GetPurchaseByIdCommand::GetPurchaseByIdCommand(PurchaseService* purchaseService , BookService* bookService)
    : m_purchaseService(purchaseService) , m_bookService(bookService)
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

        // 🟢 اضافه کردن اقلام/کتاب‌های درون این فاکتور
        QVariantList itemsList;
        for (const auto& item : purchase->getItems()) { // متد دریافت اقلام در کلاس Purchase
            QVariantMap itemMap;
            int bookId = item.getBookId();
            itemMap["bookId"] = bookId;
            QString title = m_bookService->getBookById(bookId)->getTitle();
            itemMap["title"] = title;       // عنوان کتاب
            itemMap["quantity"] = item.getQuantity(); // تعداد
            itemMap["price"] = item.getTotalPrice();       // قیمت
            itemsList.append(itemMap);
        }
        data["items"] = itemsList;

        return Response::success(CommandType::GetPurchaseById, data);
    }
    return Response::error(CommandType::GetPurchaseById, "Purchase not found");
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
    return Response::error(CommandType::EditReview ,"Failed to update review");
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
/*
Response AddBookCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QString title = params["title"].toString();
    QString author = params["author"].toString();
    Genre genre = GenreHelper::fromString(params["genre"].toString());
    QString description = params["description"].toString();
    double price = params["price"].toDouble();
    double discountPercent = params.value("discount", 0.0).toDouble();

    // ۱. دریافت آدرس کاور از پارامترها
    QString coverPath = params["coverImage"].toString();
    QString pdfFile= params["pdfFileName"].toString();




    // ۲. ارسال پارامتر جدید به متد سرویس (باید این متد را هم در سرویس خود آپدیت کنی)

    if (m_publisherService->addBook(publisherId, title, author, genre, description, price , 0, coverPath,pdfFile)) {
        QVariantMap data;
        data["title"] = title;
        data["author"] = author;
        data["coverPath"] = coverPath;
        return Response::success(CommandType::AddBook , "Book added successfully", data);
    }
    return Response::error(CommandType::AddBook ,"Failed to add book");
}
*/


Response AddBookCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QString title = params["title"].toString();
    QString author = params["author"].toString();
    Genre genre = GenreHelper::fromString(params["genre"].toString());
    QString description = params["description"].toString();
    double price = params["price"].toDouble();

    double discountPercent =
        params.value("discount", 0.0).toDouble();


    QByteArray coverBytes =
        QByteArray::fromBase64(
            params["coverData"].toByteArray()
            );

    QByteArray pdfBytes =
        QByteArray::fromBase64(
            params["pdfData"].toByteArray()
            );


    QString coverExt =
        QFileInfo(params["coverFileName"].toString())
            .suffix();


    if (coverExt.isEmpty())
        coverExt = "png";


    QDir().mkpath("covers");
    QDir().mkpath("pdfs");


    QString fileBase =
        QUuid::createUuid()
            .toString(QUuid::WithoutBraces);


    QString coverPath =
        "covers/" + fileBase + "." + coverExt;


    QString pdfPath =
        "pdfs/" + fileBase + ".pdf";


    QFile coverFile(coverPath);

    if (!coverBytes.isEmpty() &&
        coverFile.open(QIODevice::WriteOnly))
    {
        coverFile.write(coverBytes);
        coverFile.close();
    }
    else
    {
        coverPath.clear();
    }


    QFile pdfFile(pdfPath);

    if (!pdfBytes.isEmpty() &&
        pdfFile.open(QIODevice::WriteOnly))
    {
        pdfFile.write(pdfBytes);
        pdfFile.close();
    }
    else
    {
        pdfPath.clear();
    }


    if (m_publisherService->addBook(
            publisherId,
            title,
            author,
            genre,
            description,
            price,
            discountPercent,
            coverPath,
            pdfPath))
    {
        QVariantMap data;

        data["title"] = title;
        data["author"] = author;
        data["coverPath"] = coverPath;


        return Response::success(
            CommandType::AddBook,
            "Book added successfully",
            data
            );
    }


    return Response::error(
        CommandType::AddBook,
        "Failed to add book"
        );
}



/*

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
*/

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

GetPublisherBooksCommand::GetPublisherBooksCommand(PublisherService* publisherService)
    : m_publisherService(publisherService)
{
}

Response GetPublisherBooksCommand::execute(const QVariantMap& params)
{
    int publisherId = params["publisherId"].toInt();
    QVector<QSharedPointer<Book>> books = m_publisherService->getAllBooksByPublisher(publisherId);

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
        bookData["coverPath"] = book->getCoverPath();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["isDiscounted"] = book->getDiscountPercent() > 0;
        bookData["isTimed"] = book->getisTimedDiscount();
        bookData["endDate"] = book->getDiscountEndDate().toString(Qt::ISODate);
        bookData["isActive"] = book->getIsActive();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetPublisherBooks, data);
}
ApplyDiscountCommand::ApplyDiscountCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response ApplyDiscountCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();
    int bookId = params.value("bookId").toInt();
    QString discountType = params.value("discountType").toString(); // "percentage" or "fixed"
    double discountValue = params.value("discountValue").toDouble();
    bool isTimed = params.value("isTimed").toBool();
    QDateTime startDate = QDateTime::fromString(params.value("startDate").toString(), Qt::ISODate);
    QDateTime endDate = QDateTime::fromString(params.value("endDate").toString(), Qt::ISODate);


    // 2. اعتبارسنجی
    if (publisherId <= 0) {
        return Response::error(CommandType::ApplyDiscount, "Invalid publisher ID");
    }

    if (bookId <= 0) {
        return Response::error(CommandType::ApplyDiscount, "Invalid book ID");
    }

    if (discountValue <= 0) {
        return Response::error(CommandType::ApplyDiscount, "Discount value must be greater than 0");
    }

    // 3. بررسی مالکیت کتاب توسط ناشر
    QSharedPointer<Book> book = m_bookService->getBookById(bookId);
    if (!book) {
        return Response::error(CommandType::ApplyDiscount, "Book not found");
    }

    if (book->getPublisherId() != publisherId) {
        return Response::error(CommandType::ApplyDiscount, "You don't have permission to modify this book");
    }

    // 4. اعمال تخفیف
    double discountPercent = 0.0;

    if (discountType == "percentage") {
        discountPercent = discountValue;
    } else if (discountType == "fixed") {
        double originalPrice = book->getPrice();
        if (discountValue >= originalPrice) {
            return Response::error(CommandType::ApplyDiscount, "Discount amount cannot exceed the original price");
        }
        discountPercent = (discountValue / originalPrice) * 100.0;
    } else {
        return Response::error(CommandType::ApplyDiscount, "Invalid discount type");
    }

    // 5. محدودیت تخفیف
    if (discountPercent > 100) {
        return Response::error(CommandType::ApplyDiscount, "Discount cannot exceed 100%");
    }


    if (isTimed) {
        if (!startDate.isValid() || !endDate.isValid()) {
            return Response::error(
                CommandType::ApplyDiscount,
                "Invalid start or end date for timed discount"
                );
        }

        if (startDate >= endDate) {
            return Response::error(
                CommandType::ApplyDiscount,
                "Start date must be before end date"
                );
        }

        book->applyTimedDiscount(
            discountPercent,
            startDate,
            endDate
            );
    }
    else {
        book->applyDiscount(discountPercent);
    }

    // 8. ذخیره در دیتابیس
    if (!m_bookService->updateBook(book)) {
        return Response::error(CommandType::ApplyDiscount, "Failed to save discount to database");
    }

    QVariantMap data;
    data["bookId"] = bookId;
    data["title"] = book->getTitle();
    data["discountPercent"] = discountPercent;
    data["finalPrice"] = book->getFinalPrice();

    return Response::success(CommandType::ApplyDiscount, "Discount applied successfully", data);
}

RemoveDiscountCommand::RemoveDiscountCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response RemoveDiscountCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();
    int bookId = params.value("bookId").toInt();

    // 2. اعتبارسنجی
    if (publisherId <= 0) {
        return Response::error(CommandType::RemoveDiscount, "Invalid publisher ID");
    }

    if (bookId <= 0) {
        return Response::error(CommandType::RemoveDiscount, "Invalid book ID");
    }

    // 3. بررسی مالکیت کتاب توسط ناشر
    QSharedPointer<Book> book = m_bookService->getBookById(bookId);
    if (!book) {
        return Response::error(CommandType::RemoveDiscount, "Book not found");
    }

    if (book->getPublisherId() != publisherId) {
        return Response::error(CommandType::RemoveDiscount, "You don't have permission to modify this book");
    }

    // 4. بررسی اینکه کتاب تخفیف دارد
    if (!book->isDiscounted()) {
        return Response::error(CommandType::RemoveDiscount, "This book does not have any discount");
    }

    // 5. حذف تخفیف
    book->removeDiscount();

    // 6. ذخیره در دیتابیس
    if (!m_bookService->updateBook(book)) {
        return Response::error(CommandType::RemoveDiscount, "Failed to remove discount from database");
    }

    QVariantMap data;
    data["bookId"] = bookId;
    data["title"] = book->getTitle();
    data["price"] = book->getPrice();

    return Response::success(CommandType::RemoveDiscount, "Discount removed successfully", data);
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
BlockUserCommand::BlockUserCommand(UserService* userService , AdminService* adminService,ClientHandler* clientHandler)
    :  m_userService(userService) , m_adminService(adminService) , m_clientHandler(clientHandler)
{
}

Response BlockUserCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId", 0).toInt();
    QString reason = params.value("reason").toString();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::BlockUser, "Invalid user ID");
    }

    // 3. دریافت کاربر هدف
    User* targetUser = m_userService->getProfile(userId);
    if (!targetUser) {
        return Response::error(CommandType::BlockUser, "User not found");
    }

    // 4. دریافت نام ادمین (از Session)
    QString adminName = "Admin";
    if (m_clientHandler) {
        // اگر کاربر لاگین است و ادمین است، نام او را بگیر
        int adminId = m_clientHandler->getSessionUserId();
        if (adminId > 0) {
            User* adminUser = m_userService->getProfile(adminId);
            if (adminUser) {
                adminName = adminUser->getUsername();
            }
        }
    }

    // 5. اجرای عملیات مسدودسازی
    bool success = m_adminService->blockUser(userId, reason);

    // 6. ثبت در Access Log
    AccessLogEntry entry(
        QDateTime::currentDateTime(),
        adminName,
        "Block User",
        targetUser->getUsername(),
        m_clientHandler->peerAddress(),
        success ? "success" : "failed"
        );
    m_adminService->appendAccessLog(entry);

    // 7. ساخت پاسخ
    if (success) {
        QVariantMap data;
        data["userId"] = userId;
        data["username"] = targetUser->getUsername();
        data["reason"] = reason;
        return Response::success(CommandType::BlockUser, "User blocked successfully", data);
    }

    return Response::error(CommandType::BlockUser, "Failed to block user");
}

// ----- UnblockUserCommand -----
UnblockUserCommand::UnblockUserCommand(UserService* userService, AdminService* adminService, ClientHandler* clientHandler)
    : m_userService(userService), m_adminService(adminService), m_clientHandler(clientHandler)
{
}

Response UnblockUserCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId", 0).toInt();

    if (userId <= 0) {
        return Response::error(CommandType::UnblockUser, "Invalid user ID");
    }

    User* targetUser = m_userService->getProfile(userId);
    if (!targetUser) {
        return Response::error(CommandType::UnblockUser, "User not found");
    }

    QString adminName = "Admin";
    if (m_clientHandler) {
        int adminId = m_clientHandler->getSessionUserId();
        if (adminId > 0) {
            User* adminUser = m_userService->getProfile(adminId);
            if (adminUser) adminName = adminUser->getUsername();
        }
    }

    bool success = m_adminService->unblockUser(userId);

    AccessLogEntry entry(
        QDateTime::currentDateTime(),
        adminName,
        "Unblock User",
        targetUser->getUsername(),
        m_clientHandler ? m_clientHandler->peerAddress() : QString(),
        success ? "success" : "failed"
        );
    m_adminService->appendAccessLog(entry);

    if (success) {
        QVariantMap data;
        data["userId"] = userId;
        return Response::success(CommandType::UnblockUser, "User unblocked", data);
    }
    return Response::error(CommandType::UnblockUser, "Failed to unblock user");
}

// ----- DeleteUserCommand -----
DeleteUserCommand::DeleteUserCommand(UserService* userService, AdminService* adminService, ClientHandler* clientHandler)
    : m_userService(userService), m_adminService(adminService), m_clientHandler(clientHandler)
{
}


Response DeleteUserCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId", 0).toInt();

    if (userId <= 0) {
        return Response::error(CommandType::DeleteUser, "Invalid user ID");
    }

    User* targetUser = m_userService->getProfile(userId);
    if (!targetUser) {
        return Response::error(CommandType::DeleteUser, "User not found");
    }

    QString adminName = "Admin";
    if (m_clientHandler) {
        int adminId = m_clientHandler->getSessionUserId();
        if (adminId > 0) {
            User* adminUser = m_userService->getProfile(adminId);
            if (adminUser) adminName = adminUser->getUsername();
        }
    }

    bool success = m_adminService->deleteUser(userId);

    AccessLogEntry entry(
        QDateTime::currentDateTime(),
        adminName,
        "Delete User",
        targetUser->getUsername(),
        m_clientHandler ? m_clientHandler->peerAddress() : QString(),
        success ? "success" : "failed"
        );
    m_adminService->appendAccessLog(entry);

    if (success) {
        QVariantMap data;
        data["userId"] = userId;
        return Response::success(CommandType::DeleteUser, "User deleted", data);
    }
    return Response::error(CommandType::DeleteUser, "Failed to delete user");
}

// ----- GetAllUsersCommand -----
GetAllUsersCommand::GetAllUsersCommand(UserService* adminService)
    : m_adminService(adminService)
{
}

Response GetAllUsersCommand::execute(const QVariantMap& params)
{
    QString filter = params.value("filter").toString().toLower();
    QString search = params.value("search").toString().toLower();

    QVector<User*> users = m_adminService->getAllUsers();

    QVariantList userList;
    for (User* user : users) {
        // Apply filter
        if (filter == "regular" && user->isPublisher()) continue;
        if (filter == "publisher" && !user->isPublisher()) continue;
        if (filter == "blocked" && !user->isBlocked()) continue;
        if (filter == "admin" && !user->isAdmin()) continue;

        // Apply search
        if (!search.isEmpty()) {
            if (!user->getUsername().toLower().contains(search) &&
                !user->getEmail().toLower().contains(search) &&
                !user->getFullname().toLower().contains(search))
                continue;
        }

        QVariantMap userData;
        userData["id"] = user->getId();
        userData["username"] = user->getUsername();
        userData["email"] = user->getEmail();
        userData["fullName"] = user->getFullname();
        userData["role"] = user->getRoleString();
        userData["status"] =
            m_adminService->getStringStatus(user->getStatus());

        userData["registered_at"] =
            user->getCreatedAt().toString(Qt::ISODate);

        userData["last_login_at"] =
            user->getLastLogin().toString(Qt::ISODate);
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
        userData["userId"] = user->getId();
        userData["username"] = user->getUsername();
        userData["email"] = user->getEmail();
        userData["fullName"] = user->getFullname();
        userData["role"] = user->getRoleString();
        userData["blockedBy"] = QString("Admin");  // Default; populated from access log if available
        userData["reason"] = QString("Blocked by administrator");
        userData["blockedAt"] = user->getUpdatedAt().toString(Qt::ISODate);
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

//فقط اگر بعداً حذف کتاب اضافه کردی، باید هنگام حذف کتاب، فایل کاور و PDF مربوطه را هم پاک کنی.

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

GetRecentActivitiesCommand::GetRecentActivitiesCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}


GetSystemAlertsCommand::GetSystemAlertsCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response GetRecentActivitiesCommand::execute(const QVariantMap& params)
{
    int limit = params.value("limit", 10).toInt();

    QVector<QVariantMap> activities =
        m_adminService->getRecentActivities(limit);

    QVariantList list;

    for (const auto& a : activities)
    {
        list.append(a);
    }

    QVariantMap data;

    data["activities"] = list;
    data["count"] = list.size();

    return Response::success(
        CommandType::GetRecentActivities,
        "Recent activities loaded",
        data
        );
}

Response GetSystemAlertsCommand::execute(const QVariantMap& params)
{
    QStringList alerts = m_adminService->getSystemAlerts();

    QVariantList list;
    for (const QString& a : alerts) {
        list.append(a);
    }

    QVariantMap data;
    data["alerts"] = list;
    data["count"] = list.size();

    return Response::success(CommandType::GetSystemAlerts, "System alerts loaded", data);
}
GetDatabaseStatusCommand::GetDatabaseStatusCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response GetDatabaseStatusCommand::execute(const QVariantMap& params)
{
    QMap<QString, QVariant> status = m_adminService->getDatabaseStatus();

    QVariantMap data;
    for (auto it = status.begin(); it != status.end(); ++it) {
        data[it.key()] = it.value();
    }

    return Response::success(CommandType::GetDatabaseStatus, "Database status loaded", data);
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





// Commands.cpp
GetNotificationsCommand::GetNotificationsCommand(NotificationService* notificationService)
    : m_notificationService(notificationService)
{
}

Response GetNotificationsCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::GetNotifications, "Invalid user ID");
    }

    // 2. دریافت اعلان‌های کاربر
    QVector<Notification> notifications = m_notificationService->getNotificationsForUser(userId);

    // 3. ساخت لیست اعلان‌ها برای پاسخ
    QVariantList notificationList;
    for (const Notification& notif : notifications) {
        QVariantMap notifData;
        notifData["notificationId"] = notif.getNotificationId();
        notifData["type"] = NotificationService::notificationTypeToString(notif.getType());
        notifData["title"] = notif.getTitle();
        notifData["message"] = notif.getMessage();
        notifData["isRead"] = notif.getIsRead();
        notifData["createdAt"] = notif.getCreatedAt().toString(Qt::ISODate);
        notificationList.append(notifData);
    }

    // 4. دریافت تعداد اعلان‌های خوانده‌نشده
    int unreadCount = m_notificationService->getUnreadCount(userId);

    QVariantMap data;
    data["notifications"] = notificationList;
    data["count"] = notificationList.size();
    data["unreadCount"] = unreadCount;

    return Response::success(CommandType::GetNotifications, "Notifications loaded", data);

}



// Commands.cpp
MarkNotificationReadCommand::MarkNotificationReadCommand(NotificationService* notificationService)
    : m_notificationService(notificationService)
{
}

Response MarkNotificationReadCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int notificationId = params.value("notificationId").toInt();
    int userId = params.value("userId").toInt();

    // 2. اعتبارسنجی
    if (notificationId <= 0) {
        return Response::error(CommandType::MarkNotificationRead, "Invalid notification ID");
    }

    if (userId <= 0) {
        return Response::error(CommandType::MarkNotificationRead, "Invalid user ID");
    }

    // 3. علامت‌گذاری به عنوان خوانده‌شده
    bool success = m_notificationService->markAsRead(notificationId, userId);

    if (!success) {
        return Response::error(CommandType::MarkNotificationRead, "Failed to mark notification as read");
    }

    // 4. دریافت تعداد اعلان‌های خوانده‌نشده باقی‌مانده
    int unreadCount = m_notificationService->getUnreadCount(userId);
    QVariantMap data;
    data["notificationId"] = notificationId;
    data["unreadCount"] = unreadCount;

    return Response::success(CommandType::MarkNotificationRead, "Notification marked as read", data);
}


// Commands.cpp
MarkAllNotificationsReadCommand::MarkAllNotificationsReadCommand(NotificationService* notificationService)
    : m_notificationService(notificationService)
{
}

Response MarkAllNotificationsReadCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::MarkAllNotificationsRead, "Invalid user ID");
    }

    // 2. علامت‌گذاری همه اعلان‌ها به عنوان خوانده‌شده
    m_notificationService->markAllAsRead(userId);

    // 3. دریافت تعداد اعلان‌های خوانده‌نشده (که باید 0 باشد)
    int unreadCount = m_notificationService->getUnreadCount(userId);

    QVariantMap data;
    data["userId"] = userId;
    data["unreadCount"] = unreadCount;

    return Response::success(CommandType::MarkAllNotificationsRead, "All notifications marked as read", data);
}




// Commands.cpp
ClearAllNotificationsCommand::ClearAllNotificationsCommand(NotificationService* notificationService)
    : m_notificationService(notificationService)
{
}

Response ClearAllNotificationsCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::ClearAllNotifications, "Invalid user ID");
    }

    // 2. پاک کردن همه اعلان‌های کاربر
    m_notificationService->clearAllNotifications(userId);

    // 3. دریافت تعداد اعلان‌های باقی‌مانده (که باید 0 باشد)
    int unreadCount = m_notificationService->getUnreadCount(userId);
    int totalCount = m_notificationService->getNotificationsForUser(userId).size();

    QVariantMap data;
    data["userId"] = userId;
    data["unreadCount"] = unreadCount;
    data["totalCount"] = totalCount;

    return Response::success(CommandType::ClearAllNotifications, "All notifications cleared", data);
}


// Commands.cpp
GetUserShelvesCommand::GetUserShelvesCommand(UserService* userService, LibraryService* libraryService)
    : m_userService(userService)
    , m_libraryService(libraryService)
{
}

Response GetUserShelvesCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::GetUserShelves, "Invalid user ID");
    }

    // 2. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::GetUserShelves, "Library not found for this user");
    }

    // 3. دریافت قفسه‌ها
    QVector<Shelf> shelves = library->getShelves();

    // 4. ساخت لیست قفسه‌ها برای پاسخ
    QVariantList shelfList;
    for (const Shelf& shelf : shelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();

        // لیست کتاب‌های هر قفسه
        QVariantList bookIds;
        for (int bookId : shelf.getBookIds()) {
            bookIds.append(bookId);
        }
        shelfData["bookIds"] = bookIds;

        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();
    data["userId"] = userId;

    return Response::success(CommandType::GetUserShelves, "Shelves loaded successfully", data);
}


// Commands.cpp
GetBooksInShelfCommand::GetBooksInShelfCommand(LibraryService* libraryService, BookService* bookService)
    : m_libraryService(libraryService)
    , m_bookService(bookService)
{
}

Response GetBooksInShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int shelfId = params.value("shelfId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::GetBooksInShelf, "Invalid user ID");
    }

    if (shelfId <= 0) {
        return Response::error(CommandType::GetBooksInShelf, "Invalid shelf ID");
    }

    // 3. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::GetBooksInShelf, "Library not found for this user");
    }

    // 4. دریافت قفسه
    QVector<Shelf> shelves = library->getShelves();
    QVector<int> bookIds;

    bool shelfFound = false;
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            bookIds = shelf.getBookIds();
            shelfFound = true;
            break;
        }
    }

    if (!shelfFound) {
        return Response::error(CommandType::GetBooksInShelf, "Shelf not found");
    }

    // 5. دریافت اطلاعات کامل کتاب‌ها
    QVariantList bookList;
    for (int bookId : bookIds) {
        QSharedPointer<Book> book = m_bookService->getBookById(bookId);
        if (book) {
            QVariantMap bookData;
            bookData["bookId"] = book->getBookId();
            bookData["title"] = book->getTitle();
            bookData["author"] = book->getAuthor();
            bookData["genre"] = GenreHelper::toString(book->getGenre());
            bookData["description"] = book->getDescription();
            bookData["price"] = book->getPrice();
            bookData["discountPercent"] = book->getDiscountPercent();
            bookData["finalPrice"] = book->getFinalPrice();
            bookData["averageRating"] = book->getAverageRating();
            bookData["coverPath"] = book->getCoverPath();
            bookData["pdfPath"] = book->getPdfPath();
            bookData["isActive"] = book->getIsActive();
            bookList.append(bookData);
        }
    }

    // 6. دریافت نام قفسه
    QString shelfName;
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            shelfName = shelf.getName();
            break;
        }
    }

    QVariantMap data;
    data["shelfId"] = shelfId;
    data["shelfName"] = shelfName;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetBooksInShelf, "Books in shelf loaded", data);
}



CreateShelfCommand::CreateShelfCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response CreateShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    QString shelfName = params.value("name").toString().trimmed();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::CreateShelf, "Invalid user ID");
    }

    if (shelfName.isEmpty()) {
        return Response::error(CommandType::CreateShelf, "Shelf name is required");
    }

    if (shelfName.length() > 50) {
        return Response::error(CommandType::CreateShelf, "Shelf name is too long (max 50 characters)");
    }

    // 3. ایجاد قفسه
    bool success = m_libraryService->createShelf(userId, shelfName);

    if (!success) {
        return Response::error(CommandType::CreateShelf, "Failed to create shelf. Maybe a shelf with this name already exists.");
    }

    // 4. دریافت لیست قفسه‌های به‌روز شده
    QVector<Shelf> shelves = m_libraryService->getShelves(userId);

    QVariantMap data;
    data["userId"] = userId;
    data["shelfName"] = shelfName;
    data["message"] = "Shelf created successfully";

    return Response::success(CommandType::CreateShelf, "Shelf created successfully", data);
}



// Commands.cpp
DeleteShelfCommand::DeleteShelfCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response DeleteShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int shelfId = params.value("shelfId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::DeleteShelf, "Invalid user ID");
    }

    if (shelfId <= 0) {
        return Response::error(CommandType::DeleteShelf, "Invalid shelf ID");
    }

    // 3. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::DeleteShelf, "Library not found for this user");
    }

    // 4. بررسی وجود قفسه
    QVector<Shelf> shelves = library->getShelves();
    bool shelfExists = false;
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            shelfExists = true;
            break;
        }
    }

    if (!shelfExists) {
        return Response::error(CommandType::DeleteShelf, "Shelf not found");
    }

    // 5. حذف قفسه
    bool success = m_libraryService->deleteShelf(userId, shelfId);

    if (!success) {
        return Response::error(CommandType::DeleteShelf, "Failed to delete shelf");
    }

    // 6. دریافت لیست به‌روز شده قفسه‌ها (اختیاری)
    QVector<Shelf> updatedShelves = m_libraryService->getShelves(userId);

    QVariantList shelfList;
    for (const Shelf& shelf : updatedShelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();
        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["userId"] = userId;
    data["shelfId"] = shelfId;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();

    return Response::success(CommandType::DeleteShelf, "Shelf deleted successfully", data);
}




// Commands.cpp
RenameShelfCommand::RenameShelfCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response RenameShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int shelfId = params.value("shelfId").toInt();
    QString newName = params.value("newName").toString().trimmed();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::RenameShelf, "Invalid user ID");
    }

    if (shelfId <= 0) {
        return Response::error(CommandType::RenameShelf, "Invalid shelf ID");
    }

    if (newName.isEmpty()) {
        return Response::error(CommandType::RenameShelf, "New shelf name is required");
    }

    if (newName.length() > 50) {
        return Response::error(CommandType::RenameShelf, "Shelf name is too long (max 50 characters)");
    }

    // 3. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::RenameShelf, "Library not found for this user");
    }

    // 4. بررسی وجود قفسه
    QVector<Shelf> shelves = library->getShelves();
    bool shelfExists = false;
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            shelfExists = true;
            break;
        }
    }

    if (!shelfExists) {
        return Response::error(CommandType::RenameShelf, "Shelf not found");
    }

    // 5. بررسی اینکه نام جدید توسط قفسه دیگری استفاده نشده باشد
    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() != shelfId && shelf.getName() == newName) {
            return Response::error(CommandType::RenameShelf, "A shelf with this name already exists");
        }
    }

    // 6. تغییر نام قفسه
    bool success = m_libraryService->renameShelf(userId, shelfId, newName);

    if (!success) {
        return Response::error(CommandType::RenameShelf, "Failed to rename shelf");
    }

    // 7. دریافت لیست به‌روز شده قفسه‌ها
    QVector<Shelf> updatedShelves = m_libraryService->getShelves(userId);
    QVariantList shelfList;
    for (const Shelf& shelf : updatedShelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();
        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["userId"] = userId;
    data["shelfId"] = shelfId;
    data["newName"] = newName;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();

    return Response::success(CommandType::RenameShelf, "Shelf renamed successfully", data);
}




// Commands.cpp
RemoveBookFromShelfCommand::RemoveBookFromShelfCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response RemoveBookFromShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int shelfId = params.value("shelfId").toInt();
    int bookId = params.value("bookId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::RemoveBookFromShelf, "Invalid user ID");
    }

    if (shelfId <= 0) {
        return Response::error(CommandType::RemoveBookFromShelf, "Invalid shelf ID");
    }

    if (bookId <= 0) {
        return Response::error(CommandType::RemoveBookFromShelf, "Invalid book ID");
    }

    // 3. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::RemoveBookFromShelf, "Library not found for this user");
    }

    // 4. بررسی وجود قفسه
    QVector<Shelf> shelves = library->getShelves();
    bool shelfExists = false;
    bool bookInShelf = false;

    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == shelfId) {
            shelfExists = true;
            if (shelf.getBookIds().contains(bookId)) {
                bookInShelf = true;
            }
            break;
        }
    }

    if (!shelfExists) {
        return Response::error(CommandType::RemoveBookFromShelf, "Shelf not found");
    }

    if (!bookInShelf) {
        return Response::error(CommandType::RemoveBookFromShelf, "Book not found in this shelf");
    }

    // 5. حذف کتاب از قفسه
    bool success = m_libraryService->removeBookFromShelf(userId, shelfId, bookId);

    if (!success) {
        return Response::error(CommandType::RemoveBookFromShelf, "Failed to remove book from shelf");
    }

    // 6. دریافت لیست به‌روز شده قفسه‌ها
    QVector<Shelf> updatedShelves = m_libraryService->getShelves(userId);
    QVariantList shelfList;
    for (const Shelf& shelf : updatedShelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();

        QVariantList bookIds;
        for (int id : shelf.getBookIds()) {
            bookIds.append(id);
        }
        shelfData["bookIds"] = bookIds;
        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["userId"] = userId;
    data["shelfId"] = shelfId;
    data["bookId"] = bookId;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();

    return Response::success(CommandType::RemoveBookFromShelf, "Book removed from shelf successfully", data);
}
// Commands.cpp
AddBookToShelfCommand::AddBookToShelfCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response AddBookToShelfCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int shelfId = params.value("shelfId").toInt();
    int bookId = params.value("bookId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::AddBookToShelf, "Invalid user ID");
    }
    if (shelfId <= 0) {
        return Response::error(CommandType::AddBookToShelf, "Invalid shelf ID");
    }
    if (bookId <= 0) {
        return Response::error(CommandType::AddBookToShelf, "Invalid book ID");
    }

    // 3. افزودن کتاب به قفسه
    bool success = m_libraryService->addBookToShelf(userId, shelfId, bookId);

    if (!success) {
        return Response::error(CommandType::AddBookToShelf, "Failed to add book to shelf");
    }

    // 4. دریافت لیست به‌روز شده قفسه‌ها (اختیاری)
    QVector<Shelf> shelves = m_libraryService->getShelves(userId);
    QVariantList shelfList;
    for (const Shelf& shelf : shelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();

        QVariantList bookIds;
        for (int id : shelf.getBookIds()) {
            bookIds.append(id);
        }
        shelfData["bookIds"] = bookIds;
        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["userId"] = userId;
    data["shelfId"] = shelfId;
    data["bookId"] = bookId;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();

    return Response::success(CommandType::AddBookToShelf, "Book added to shelf successfully", data);
}



// Commands.cpp
MoveBookBetweenShelvesCommand::MoveBookBetweenShelvesCommand(LibraryService* libraryService)
    : m_libraryService(libraryService)
{
}

Response MoveBookBetweenShelvesCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int fromShelfId = params.value("fromShelfId").toInt();
    int toShelfId = params.value("toShelfId").toInt();
    int bookId = params.value("bookId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Invalid user ID");
    }

    if (fromShelfId <= 0) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Invalid source shelf ID");
    }

    if (toShelfId <= 0) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Invalid destination shelf ID");
    }

    if (bookId <= 0) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Invalid book ID");
    }

    if (fromShelfId == toShelfId) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Source and destination shelves are the same");
    }

    // 3. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Library not found for this user");
    }

    // 4. بررسی وجود هر دو قفسه و کتاب
    QVector<Shelf> shelves = library->getShelves();
    bool fromShelfExists = false;
    bool toShelfExists = false;
    bool bookInFromShelf = false;

    for (const Shelf& shelf : shelves) {
        if (shelf.getShelfId() == fromShelfId) {
            fromShelfExists = true;
            if (shelf.getBookIds().contains(bookId)) {
                bookInFromShelf = true;
            }
        }
        if (shelf.getShelfId() == toShelfId) {
            toShelfExists = true;
        }
    }

    if (!fromShelfExists) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Source shelf not found");
    }

    if (!toShelfExists) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Destination shelf not found");
    }

    if (!bookInFromShelf) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Book not found in source shelf");
    }

    // 5. انتقال کتاب بین قفسه‌ها
    bool success = m_libraryService->moveBookBetweenShelves(userId, fromShelfId, toShelfId, bookId);

    if (!success) {
        return Response::error(CommandType::MoveBookBetweenShelves, "Failed to move book between shelves");
    }

    // 6. دریافت لیست به‌روز شده قفسه‌ها
    QVector<Shelf> updatedShelves = m_libraryService->getShelves(userId);
    QVariantList shelfList;
    for (const Shelf& shelf : updatedShelves) {
        QVariantMap shelfData;
        shelfData["shelfId"] = shelf.getShelfId();
        shelfData["name"] = shelf.getName();
        shelfData["bookCount"] = shelf.getBookCount();

        QVariantList bookIds;
        for (int id : shelf.getBookIds()) {
            bookIds.append(id);
        }
        shelfData["bookIds"] = bookIds;
        shelfList.append(shelfData);
    }

    QVariantMap data;
    data["userId"] = userId;
    data["bookId"] = bookId;
    data["fromShelfId"] = fromShelfId;
    data["toShelfId"] = toShelfId;
    data["shelves"] = shelfList;
    data["count"] = shelfList.size();

    return Response::success(CommandType::MoveBookBetweenShelves, "Book moved successfully", data);
}



// Commands.cpp
GetBestSellersCommand::GetBestSellersCommand(BookService* bookService , UserService* userService)
    : m_bookService(bookService) , m_userService(userService)
{
}

Response GetBestSellersCommand::execute(const QVariantMap& params)
{
    // 1. دریافت تعداد (limit) از پارامترها (پیش‌فرض 10)
    int limit = params.value("limit", 10).toInt();
    int userId = params["userId"].toInt();

    if (limit <= 0) {
        limit = 10;
    }

    // 2. دریافت کتاب‌های پرفروش از BookService
    QVector<QSharedPointer<Book>> books = m_bookService->getPopularBooks(limit);

    // 3. ساخت لیست کتاب‌ها برای پاسخ
    QVariantList bookList;
    for (QSharedPointer<Book> book : books) {
        QVariantMap bookData;
        int bookId = book->getBookId();
        bookData["bookId"] = bookId;
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["description"] = book->getDescription();
        bookData["price"] = book->getPrice();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        bookData["coverPath"] = book->getCoverPath();
        bookData["pdfPath"] = book->getPdfPath();
        bookData["isActive"] = book->getIsActive();
        bookData["discountPercent"] = book->getDiscountPercent();

        // اطلاعات ناشر (اختیاری)
        // int publisherId = book->getPublisherId();
        // Publisher* publisher = m_userService->getPublisherById(publisherId);
        // if (publisher) {
        //     bookData["publisherName"] = publisher->getPublisherName();
        // }
        if (userId > 0) {
            bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
            bookData["isFavorite"] = isFavorite;
        }

        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetBestSellers, "Best sellers loaded", data);

}




GetBookCoverCommand::GetBookCoverCommand(
    BookService* bookService)
    : m_bookService(bookService)
{
}


Response GetBookCoverCommand::execute(
    const QVariantMap& params)
{
    int bookId =
        params["bookId"].toInt();


    QSharedPointer<Book> book =
        m_bookService->getBookById(bookId);


    if (!book)
    {
        return Response::error(
            CommandType::GetBookCover,
            "Book not found"
            );
    }


    QString path =
        book->getCoverPath();


    if (path.isEmpty())
    {
        return Response::error(
            CommandType::GetBookCover,
            "No cover available"
            );
    }


    QFile file(path);


    if (!file.open(QIODevice::ReadOnly))
    {
        return Response::error(CommandType::GetBookCover,"Cover file missing on server");
    }


    QVariantMap data;

    data["bookId"] = bookId;

    data["coverData"] =
        QString::fromLatin1(
            file.readAll().toBase64()
            );


    return Response::success(
        CommandType::GetBookCover,
        data
        );
}


AddFavoriteBookCommand::AddFavoriteBookCommand(UserService* userService)
    : m_userService(userService)
{
}


Response AddFavoriteBookCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId").toInt();
    int bookId = params.value("bookId").toInt();

    if (userId <= 0 || bookId <= 0) {
        return Response::error(CommandType::AddFavoriteBook, "Invalid user ID or book ID");
    }

    bool success = m_userService->addFavoriteBook(userId, bookId);

    if (success) {
        QVariantMap data;
        data["userId"] = userId;
        data["bookId"] = bookId;
        return Response::success(CommandType::AddFavoriteBook, "Book added to favorites", data);
    }
    return Response::error(CommandType::AddFavoriteBook, "Failed to add book to favorites");
}



// Commands.cpp
GetFavoriteBooksCommand::GetFavoriteBooksCommand(UserService* userService, BookService* bookService)
    : m_userService(userService)
    , m_bookService(bookService)
{
}

Response GetFavoriteBooksCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::GetFavoriteBooks, "Invalid user ID");
    }

    // 2. دریافت لیست کتاب‌های علاقه‌مندی کاربر
    QVector<int> favoriteBookIds = m_userService->getFavoriteBooks(userId);

    if (favoriteBookIds.isEmpty()) {
        QVariantMap data;
        data["books"] = QVariantList();
        data["count"] = 0;
        return Response::success(CommandType::GetFavoriteBooks, "No favorite books found", data);
    }

    // 3. دریافت اطلاعات کامل کتاب‌ها
    QVariantList bookList;
    for (int bookId : favoriteBookIds) {
        QSharedPointer<Book> book = m_bookService->getBookById(bookId);
        if (book) {
            QVariantMap bookData;
            bookData["bookId"] = book->getBookId();
            bookData["title"] = book->getTitle();
            bookData["author"] = book->getAuthor();
            bookData["genre"] = GenreHelper::toString(book->getGenre());
            bookData["description"] = book->getDescription();
            bookData["price"] = book->getPrice();
            bookData["discountPercent"] = book->getDiscountPercent();
            bookData["finalPrice"] = book->getFinalPrice();
            bookData["averageRating"] = book->getAverageRating();
            bookData["coverPath"] = book->getCoverPath();
            bookData["pdfPath"] = book->getPdfPath();
            bookData["isActive"] = book->getIsActive();
            if (userId > 0) {
                bool isFavorite = m_userService->isFavoriteBook(userId, bookId);
                bookData["isFavorite"] = isFavorite;
            }
            bookList.append(bookData);
        }
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetFavoriteBooks, "Favorite books loaded", data);
}


// Commands.cpp
RemoveFavoriteBookCommand::RemoveFavoriteBookCommand(UserService* userService)
    : m_userService(userService)
{
}

Response RemoveFavoriteBookCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int userId = params.value("userId").toInt();
    int bookId = params.value("bookId").toInt();

    // 2. اعتبارسنجی
    if (userId <= 0) {
        return Response::error(CommandType::RemoveFavoriteBook, "Invalid user ID");
    }

    if (bookId <= 0) {
        return Response::error(CommandType::RemoveFavoriteBook, "Invalid book ID");
    }

    // 3. حذف کتاب از علاقه‌مندی‌ها
    bool success = m_userService->removeFavoriteBook(userId, bookId);

    if (!success) {
        return Response::error(CommandType::RemoveFavoriteBook, "Failed to remove book from favorites");
    }

    // 4. دریافت لیست به‌روز شده (اختیاری)
    QVector<int> updatedFavorites = m_userService->getFavoriteBooks(userId);

    QVariantMap data;
    data["userId"] = userId;
    data["bookId"] = bookId;
    data["favoriteBooks"] = QVariant::fromValue(updatedFavorites);
    data["count"] = updatedFavorites.size();

    return Response::success(CommandType::RemoveFavoriteBook, "Book removed from favorites", data);
}



GetUserLibraryCommand::GetUserLibraryCommand(LibraryService* libraryService, BookService* bookService)
    : m_libraryService(libraryService)
    , m_bookService(bookService)
{
}

Response GetUserLibraryCommand::execute(const QVariantMap& params)
{
    // 1. دریافت userId از پارامترها
    int userId = params.value("userId").toInt();

    if (userId <= 0) {
        return Response::error(CommandType::GetUserLibrary, "Invalid user ID");
    }

    // 2. دریافت کتابخانه کاربر
    QSharedPointer<Library> library = m_libraryService->getLibraryByUserId(userId);
    if (!library) {
        return Response::error(CommandType::GetUserLibrary, "Library not found for this user");
    }

    // 3. دریافت لیست کتاب‌های خریداری‌شده
    QVector<int> ownedBookIds = library->getOwnedBooks();

    if (ownedBookIds.isEmpty()) {
        QVariantMap data;
        data["books"] = QVariantList();
        data["count"] = 0;
        return Response::success(CommandType::GetUserLibrary, "No books in library", data);
    }

    // 4. دریافت اطلاعات کامل کتاب‌ها
    QVariantList bookList;
    for (int bookId : ownedBookIds) {
        QSharedPointer<Book> book = m_bookService->getBookById(bookId);
        if (book) {
            QVariantMap bookData;
            bookData["bookId"] = book->getBookId();
            bookData["title"] = book->getTitle();
            bookData["author"] = book->getAuthor();
            bookData["genre"] = GenreHelper::toString(book->getGenre());
            bookData["description"] = book->getDescription();
            bookData["price"] = book->getPrice();
            bookData["discountPercent"] = book->getDiscountPercent();
            bookData["finalPrice"] = book->getFinalPrice();
            bookData["averageRating"] = book->getAverageRating();
            bookData["coverPath"] = book->getCoverPath();
            bookData["pdfPath"] = book->getPdfPath();
            bookData["isActive"] = book->getIsActive();
            bookList.append(bookData);
        }
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetUserLibrary, "Library loaded", data);
}



// Commands.cpp
GetSalesTrendCommand::GetSalesTrendCommand(BookService* bookService, PurchaseService* purchaseService)
    : m_bookService(bookService)
    , m_purchaseService(purchaseService)
{
}

Response GetSalesTrendCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();
    QString period = params.value("period", "Daily").toString(); // Daily, Weekly, Monthly
    int limit = params.value("limit", 30).toInt();

    if (publisherId <= 0) {
        return Response::error(CommandType::GetSalesTrend, "Invalid publisher ID");
    }

    // 2. دریافت کتاب‌های ناشر
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(publisherId);
    if (books.isEmpty()) {
        QVariantMap data;
        data["labels"] = QVariantList();
        data["sales"] = QVariantList();
        data["revenue"] = QVariantList();
        return Response::success(CommandType::GetSalesTrend, "No books found for this publisher", data);
    }

    // 3. ایجاد لیست خریدها برای کتاب‌های ناشر
    QVector<int> bookIds;
    for (const auto& book : books) {
        bookIds.append(book->getBookId());
    }

    QVector<QSharedPointer<Purchase>> allPurchases = m_purchaseService->getAllPurchases();

    // 4. فیلتر کردن خریدها بر اساس کتاب‌های ناشر
    QVector<QSharedPointer<Purchase>> publisherPurchases;
    for (QSharedPointer<Purchase> purchase : allPurchases) {
        for (const CartItem& item : purchase->getItems()) {
            if (bookIds.contains(item.getBookId())) {
                publisherPurchases.append(purchase);
                break;
            }
        }
    }

    // 5. گروه‌بندی بر اساس بازه زمانی
    QMap<QString, QPair<int, double>> groupedData;

    for (QSharedPointer<Purchase> purchase : publisherPurchases) {
        QString key;
        QDateTime date = purchase->getPurchasedAt();

        if (period == "Daily") {
            key = date.toString("yyyy-MM-dd");
        } else if (period == "Weekly") {
            // شروع هفته (دوشنبه)
            int weekNumber = date.date().weekNumber();
            key = QString("%1-W%2").arg(date.date().year()).arg(weekNumber, 2, 10, QChar('0'));
        } else if (period == "Monthly") {
            key = date.toString("yyyy-MM");
        } else {
            key = date.toString("yyyy-MM-dd");
        }

        // محاسبه فروش فقط برای کتاب‌های ناشر
        double revenue = 0.0;
        int salesCount = 0;
        for (const CartItem& item : purchase->getItems()) {
            if (bookIds.contains(item.getBookId())) {
                revenue += item.getTotalDiscountedPrice();
                salesCount += item.getQuantity();
            }
        }

        if (groupedData.contains(key)) {
            groupedData[key].first += salesCount;
            groupedData[key].second += revenue;
        } else {
            groupedData[key] = qMakePair(salesCount, revenue);
        }
    }

    // 6. مرتب‌سازی بر اساس تاریخ
    QList<QString> sortedKeys = groupedData.keys();
    std::sort(sortedKeys.begin(), sortedKeys.end());

    // 7. محدود کردن تعداد داده‌ها
    if (sortedKeys.size() > limit) {
        sortedKeys = sortedKeys.mid(sortedKeys.size() - limit);
    }

    // 8. ساخت پاسخ
    QVariantList labels;
    QVariantList salesData;
    QVariantList revenueData;

    for (const QString& key : sortedKeys) {
        labels.append(key);
        salesData.append(groupedData[key].first);
        revenueData.append(groupedData[key].second);
    }

    QVariantMap data;
    data["labels"] = labels;
    data["sales"] = salesData;
    data["revenue"] = revenueData;
    data["period"] = period;
    data["count"] = labels.size();

    return Response::success(CommandType::GetSalesTrend, "Sales trend loaded", data);
}


// Commands.cpp
GetBookRatingsChartCommand::GetBookRatingsChartCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBookRatingsChartCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();

    if (publisherId <= 0) {
        return Response::error(CommandType::GetBookRatingsChart, "Invalid publisher ID");
    }

    // 2. دریافت کتاب‌های ناشر
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(publisherId);

    if (books.isEmpty()) {
        QVariantMap data;
        data["bookTitles"] = QVariantList();
        data["ratings"] = QVariantList();
        data["count"] = 0;
        return Response::success(CommandType::GetBookRatingsChart, "No books found for this publisher", data);
    }
    QVariantList bookTitles;
    QVariantList ratings;
    std::sort(books.begin(), books.end(),
              [](const QSharedPointer<Book>& a, const QSharedPointer<Book>& b) {
                  return a->getAverageRating() > b->getAverageRating();
              });

    int maxBooks = qMin(10, books.size());

    for (int i = 0; i < maxBooks; ++i) {
        QSharedPointer<Book> book = books[i];
        bookTitles.append(book->getTitle());
        ratings.append(book->getAverageRating());
    }

    QVariantMap data;
    data["bookTitles"] = bookTitles;
    data["ratings"] = ratings;
    data["count"] = bookTitles.size();

    return Response::success(CommandType::GetBookRatingsChart, "Book ratings chart loaded", data);
}


GetTopSellingBooksCommand::GetTopSellingBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetTopSellingBooksCommand::execute(const QVariantMap& params)
{
    int publisherId = params.value("publisherId").toInt();
    int limit = params.value("limit", 5).toInt();

    if (publisherId <= 0) {
        return Response::error(CommandType::GetTopSellingBooks, "Invalid publisher ID");
    }

    if (limit <= 0) {
        limit = 5;
    }

    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(publisherId);

    if (books.isEmpty()) {
        QVariantMap data;
        data["books"] = QVariantList();
        data["count"] = 0;
        return Response::success(CommandType::GetTopSellingBooks, "No books found for this publisher", data);
    }

    std::sort(books.begin(), books.end(),
              [](const QSharedPointer<Book>& a, const QSharedPointer<Book>& b) {
                  return a->getSalesCount() > b->getSalesCount();
              });

    int count = qMin(limit, books.size());
    QVariantList bookList;
    for (int i = 0; i < count; ++i) {
        QSharedPointer<Book> book = books[i];
        QVariantMap bookData;
        bookData["rank"] = i + 1;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["salesCount"] = book->getSalesCount();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();
        double revenue = book->getFinalPrice() * book->getSalesCount();
        bookData["revenue"] = revenue;

        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetTopSellingBooks, "Top selling books loaded", data);
}


GetBottomSellingBooksCommand::GetBottomSellingBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetBottomSellingBooksCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();
    int limit = params.value("limit", 5).toInt();

    if (publisherId <= 0) {
        return Response::error(CommandType::GetBottomSellingBooks, "Invalid publisher ID");
    }

    if (limit <= 0) {
        limit = 5;
    }

    // 2. دریافت کتاب‌های ناشر
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(publisherId);

    if (books.isEmpty()) {
        QVariantMap data;
        data["books"] = QVariantList();
        data["count"] = 0;
        return Response::success(CommandType::GetBottomSellingBooks, "No books found for this publisher", data);
    }

    // 3. مرتب‌سازی بر اساس فروش (صعودی)
    std::sort(books.begin(), books.end(),
              [](const QSharedPointer<Book>& a, const QSharedPointer<Book>& b) {
                  return a->getSalesCount() < b->getSalesCount();
              });

    // 4. محدود کردن تعداد
    int count = qMin(limit, books.size());

    // 5. ساخت لیست کتاب‌های کم‌فروش
    QVariantList bookList;
    for (int i = 0; i < count; ++i) {
        QSharedPointer<Book> book = books[i];
        QVariantMap bookData;
        bookData["rank"] = i + 1;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["salesCount"] = book->getSalesCount();
        bookData["price"] = book->getPrice();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["coverPath"] = book->getCoverPath();

        // محاسبه درآمد (قیمت نهایی × تعداد فروش)
        double revenue = book->getFinalPrice() * book->getSalesCount();
        bookData["revenue"] = revenue;

        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();

    return Response::success(CommandType::GetBottomSellingBooks, "Bottom selling books loaded", data);
}



// Commands.cpp
GetSalesOverviewCommand::GetSalesOverviewCommand(BookService* bookService, PurchaseService* purchaseService)
    : m_bookService(bookService)
    , m_purchaseService(purchaseService)
{
}

Response GetSalesOverviewCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int publisherId = params.value("publisherId").toInt();

    if (publisherId <= 0) {
        return Response::error(CommandType::GetSalesOverview, "Invalid publisher ID");
    }

    // 2. دریافت کتاب‌های ناشر
    QVector<QSharedPointer<Book>> books = m_bookService->getBooksByPublisher(publisherId);

    if (books.isEmpty()) {
        QVariantMap data;
        data["totalBooks"] = 0;
        data["totalSales"] = 0;
        data["totalRevenue"] = 0.0;
        data["averageRating"] = 0.0;
        data["bestSeller"] = QVariantMap();
        data["worstSeller"] = QVariantMap();
        data["periods"] = QVariantList();
        return Response::success(CommandType::GetSalesOverview, "No books found for this publisher", data);
    }

    // 3. محاسبه آمار کلی
    int totalBooks = books.size();
    int totalSales = 0;
    double totalRevenue = 0.0;
    double totalRating = 0.0;
    int ratedBooks = 0;

    // شناسایی پرفروش‌ترین و کم‌فروش‌ترین کتاب
    QSharedPointer<Book> bestSeller = books[0];
    QSharedPointer<Book> worstSeller = books[0];

    for (const auto& book : books) {
        // فروش کل
        totalSales += book->getSalesCount();

        // درآمد کل (قیمت نهایی × تعداد فروش)
        totalRevenue += book->getFinalPrice() * book->getSalesCount();

        // میانگین امتیاز
        if (book->getAverageRating() > 0) {
            totalRating += book->getAverageRating();
            ratedBooks++;
        }

        // پرفروش‌ترین
        if (book->getSalesCount() > bestSeller->getSalesCount()) {
            bestSeller = book;
        }

        // کم‌فروش‌ترین
        if (book->getSalesCount() < worstSeller->getSalesCount()) {
            worstSeller = book;
        }
    }

    // 4. دریافت خلاصه دوره‌های زمانی از PurchaseService
    QVariantList periodSummaries = m_purchaseService->getPeriodSummaries(publisherId);

    // 5. ساخت پاسخ
    QVariantMap data;
    data["totalBooks"] = totalBooks;
    data["totalSales"] = totalSales;
    data["totalRevenue"] = totalRevenue;
    data["averageRating"] = ratedBooks > 0 ? totalRating / ratedBooks : 0.0;

    // اطلاعات پرفروش‌ترین کتاب
    QVariantMap bestSellerData;
    bestSellerData["bookId"] = bestSeller->getBookId();
    bestSellerData["title"] = bestSeller->getTitle();
    bestSellerData["author"] = bestSeller->getAuthor();
    bestSellerData["salesCount"] = bestSeller->getSalesCount();
    bestSellerData["revenue"] = bestSeller->getFinalPrice() * bestSeller->getSalesCount();
    data["bestSeller"] = bestSellerData;

    // اطلاعات کم‌فروش‌ترین کتاب
    QVariantMap worstSellerData;
    worstSellerData["bookId"] = worstSeller->getBookId();
    worstSellerData["title"] = worstSeller->getTitle();
    worstSellerData["author"] = worstSeller->getAuthor();
    worstSellerData["salesCount"] = worstSeller->getSalesCount();
    worstSellerData["revenue"] = worstSeller->getFinalPrice() * worstSeller->getSalesCount();
    data["worstSeller"] = worstSellerData;

    // خلاصه دوره‌های زمانی
    data["periods"] = periodSummaries;

    return Response::success(CommandType::GetSalesOverview, "Sales overview loaded", data);
}


CheckBookOwnershipCommand::CheckBookOwnershipCommand(LibraryService* libraryService)
    : m_libraryService(libraryService) {}

Response CheckBookOwnershipCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId", -1).toInt();
    int bookId = params.value("bookId", -1).toInt();

    if (userId <= 0 || bookId <= 0) {
        return Response::error(CommandType::CheckBookOwnership, "Invalid userId or bookId");
    }

    // ASSUMPTION: LibraryService exposes isBookOwned(userId, bookId).
    // Adjust this one line if the real method name/signature differs.
    bool owned = m_libraryService->isBookOwned(userId, bookId);


    QVariantMap data;
    data["bookId"] = bookId;
    data["isOwned"] = owned;
    return Response::success(CommandType::CheckBookOwnership, data);
}



//admin section


ToggleUserActiveCommand::ToggleUserActiveCommand(UserService* userService)
    : m_userService(userService)
{
}

Response ToggleUserActiveCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId").toInt();
    if (userId <= 0) {
        return Response::error(CommandType::ToggleUserActiveStatus, "Invalid user ID");
    }

    User* user = m_userService->getProfile(userId);
    if (!user) {
        return Response::error(CommandType::ToggleUserActiveStatus, "User not found");
    }

    bool nowActive;
    if (user->getStatus() == AccountStatus::Active) {
        nowActive = m_userService->deactivateUser(userId);   // add if missing
    } else {
        nowActive = m_userService->activateUser(userId);     // add if missing
    }

    if (!nowActive) {
        return Response::error(CommandType::ToggleUserActiveStatus, "Failed to update user status");
    }

    QVariantMap data;
    data["userId"] = userId;
    data["status"] = m_userService->getStringStatus(m_userService->getProfile(userId)->getStatus());
    return Response::success(CommandType::ToggleUserActiveStatus, "User status updated", data);
}

GetAdminAccessLogCommand::GetAdminAccessLogCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response GetAdminAccessLogCommand::execute(const QVariantMap& params)
{
    int limit = params.value("limit", 50).toInt();

    QVector<AccessLogEntry> records = m_adminService->getAccessLogs();

    QVariantList list;
    int count = 0;
    for (const auto& r : records) {
        QVariantMap m;
        m["timestamp"]  = r.timestamp.toString(Qt::ISODate);
        m["adminName"]  = r.adminName;
        m["action"]     = r.action;
        m["targetUser"] = r.targetUser;
        m["ipAddress"]  = r.ipAddress;
        m["status"]     = r.status;
        list.append(m);
        if (++count >= limit) break;
    }

    QVariantMap data;
    data["log"] = list;
    data["count"] = list.size();
    return Response::success(CommandType::GetAdminAccessLog, data);
}

// Commands.cpp
GetAdminBooksCommand::GetAdminBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetAdminBooksCommand::execute(const QVariantMap& params)
{
    QString search   = params.value("search").toString();
    int publisherId  = params.value("publisherId", -1).toInt();
    QString status   = params.value("status").toString(); // "", "active", "inactive", "flagged"

    QVector<QSharedPointer<Book>> books = search.isEmpty()
                                              ? m_bookService->getBookRepo()->getAllBooks()             // add if missing — trivial repo passthrough
                                              : m_bookService->searchBooks(search);

    QVariantList bookList;
    for (const auto& book : books) {
        if (publisherId > 0 && book->getPublisherId() != publisherId) continue;
        if (status == "active" && !book->getIsActive()) continue;
        if (status == "inactive" && book->getIsActive()) continue;
        if (status == "flagged" && !book->getIsFlagged()) continue;

        QVariantMap bookData;
        bookData["bookId"]        = book->getBookId();
        bookData["title"]         = book->getTitle();
        bookData["author"]        = book->getAuthor();
        bookData["publisherId"]   = book->getPublisherId();
        bookData["price"]         = book->getFinalPrice();
        bookData["status"]        = book->getIsFlagged() ? "flagged"
                                                  : (book->getIsActive() ? "active" : "inactive");
        bookData["salesCount"]    = book->getSalesCount();
        bookData["averageRating"] = book->getAverageRating();
        bookList.append(bookData);
    }

    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    return Response::success(CommandType::GetAdminBooks, data);
}



// Commands.cpp
FlagBookCommand::FlagBookCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response FlagBookCommand::execute(const QVariantMap& params)
{
    int bookId = params.value("bookId").toInt();
    if (bookId <= 0) {
        return Response::error(CommandType::FlagBook, "Invalid book ID");
    }

    QSharedPointer<Book> book = m_bookService->getBookById(bookId);
    if (!book) {
        return Response::error(CommandType::FlagBook, "Book not found");
    }

    book->setIsFlagged(true);   // new setter mirroring applyDiscount()/removeDiscount() style
    if (!m_bookService->updateBook(book)) {
        return Response::error(CommandType::FlagBook, "Failed to flag book");
    }

    QVariantMap data;
    data["bookId"] = bookId;
    return Response::success(CommandType::FlagBook, "Book flagged for review", data);
}


UnflagBookCommand::UnflagBookCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response UnflagBookCommand::execute(const QVariantMap& params)
{
    // 1. دریافت پارامترها
    int bookId = params.value("bookId", 0).toInt();

    // 2. اعتبارسنجی
    if (bookId <= 0) {
        return Response::error(CommandType::UnflagBook, "Invalid book ID");
    }

    // 3. دریافت کتاب
    QSharedPointer<Book> book = m_bookService->getBookById(bookId);
    if (!book) {
        return Response::error(CommandType::UnflagBook, "Book not found");
    }

    // 4. بررسی اینکه کتاب فلگ شده باشد
    if (!book->getIsFlagged()) {
        return Response::error(CommandType::UnflagBook, "Book is not flagged");
    }

    // 5. حذف فلگ
    book->setIsFlagged(false);
    book->setUpdatedAt(QDateTime::currentDateTime());

    // 6. ذخیره در دیتابیس
    if (!m_bookService->updateBook(book)) {
        return Response::error(CommandType::UnflagBook, "Failed to unflag book");
    }

    // 7. ساخت پاسخ
    QVariantMap data;
    data["bookId"] = bookId;
    data["title"] = book->getTitle();
    data["flagged"] = false;

    return Response::success(CommandType::UnflagBook, "Flag removed from book", data);
}




// Commands.cpp
GetAdminReviewsCommand::GetAdminReviewsCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response GetAdminReviewsCommand::execute(const QVariantMap& params)
{
    QString status  = params.value("status").toString();     // "", pending, approved, rejected, flagged
    int rating      = params.value("rating", 0).toInt();      // 0 = all
    QString search  = params.value("search").toString();
    int limit       = params.value("limit", -1).toInt();
    int bookIdFilter = params.value("bookId", -1).toInt();

    QVector<QSharedPointer<Review>> reviews = m_reviewService->getAllReviews(); // new passthrough

    QVariantList reviewList;
    for (const auto& review : reviews) {
        if (!status.isEmpty() && review->getStatus() != status) continue;
        if (rating > 0 && review->getRating() != rating) continue;
        if (!search.isEmpty() &&
            !review->getText().contains(search, Qt::CaseInsensitive)) continue;
        if (bookIdFilter > 0 && review->getBookId() != bookIdFilter) { continue; }

        QVariantMap r;
        r["reviewId"]   = review->getReviewId();
        r["bookId"]     = review->getBookId();
        r["userId"]     = review->getUserId();
        r["rating"]     = review->getRating();
        r["text"]       = review->getText();
        r["status"]     = review->getStatus();
        r["isFlagged"]  = review->getIsFlagged();
        r["createdAt"]  = review->getCreatedAt().toString(Qt::ISODate);
        reviewList.append(r);

        if (limit > 0 && reviewList.size() >= limit) break;
    }

    QVariantMap data;
    data["reviews"] = reviewList;
    data["count"] = reviewList.size();
    return Response::success(CommandType::GetAdminReviews, data);
}

// --- Approve / Reject / Flag follow the exact same shape ---

ApproveReviewCommand::ApproveReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response ApproveReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params.value("reviewId").toInt();
    if (!m_reviewService->setReviewStatus(reviewId, "approved")) {  // new
        return Response::error(CommandType::ApproveReview, "Failed to approve review");
    }
    return Response::success(CommandType::ApproveReview, "Review approved");
}

RejectReviewCommand::RejectReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response RejectReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params.value("reviewId").toInt();
    QString reason = params.value("reason").toString();
    if (!m_reviewService->setReviewStatus(reviewId, "rejected")) {
        return Response::error(CommandType::RejectReview, "Failed to reject review");
    }
    return Response::success(CommandType::RejectReview, "Review rejected");
}

FlagReviewCommand::FlagReviewCommand(ReviewService* reviewService)
    : m_reviewService(reviewService)
{
}

Response FlagReviewCommand::execute(const QVariantMap& params)
{
    int reviewId = params.value("reviewId").toInt();
    if (!m_reviewService->setReviewFlagged(reviewId, true)) {
        return Response::error(CommandType::FlagReview, "Failed to flag review");
    }
    return Response::success(CommandType::FlagReview, "Review flagged");
}


GetServerRuntimeStatusCommand::GetServerRuntimeStatusCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response GetServerRuntimeStatusCommand::execute(const QVariantMap& params)
{
    // AdminService should ask ClientHandler's connection registry for
    // this rather than owning it — see caveat below.
    QVariantMap data;
    data["online"]       = true;
    data["onlineUsers"]  = m_adminService->getOnlineUserCount();      // new
    data["dbConnected"]  = m_adminService->isDatabaseConnected();     // new
    data["uptime"]       = m_adminService->getServerUptimeString();    // new
    return Response::success(CommandType::GetServerRuntimeStatus, data);
}

// Commands.cpp
BroadcastMessageCommand::BroadcastMessageCommand(ClientHandler* clientHandler)
    : m_clientHandler(clientHandler)
{
}

Response BroadcastMessageCommand::execute(const QVariantMap& params)
{
    QString message = params.value("message").toString();
    if (message.trimmed().isEmpty()) {
        return Response::error(CommandType::BroadcastMessage, "Message is empty");
    }

    // Requires the ClientHandler/Server layer to expose a way to push
    // an unsolicited Response to every connected socket — this is the
    // one feature that needs new server "push" plumbing (see caveat).
    m_clientHandler->broadcastToAllClients(
        Response::success(CommandType::BroadcastMessage, message));

    return Response::success(CommandType::BroadcastMessage, "Message broadcasted");
}

BackupDatabaseCommand::BackupDatabaseCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response BackupDatabaseCommand::execute(const QVariantMap& params)
{
    QString path = m_adminService->backupDatabase();
    if (path.isEmpty()) {
        return Response::error(CommandType::BackupDatabase, "Backup failed");
    }
    QVariantMap data; data["path"] = path;
    return Response::success(CommandType::BackupDatabase, "Backup completed", data);
}

ClearServerCacheCommand::ClearServerCacheCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response ClearServerCacheCommand::execute(const QVariantMap& params)
{
    m_adminService->clearCaches(); // new — e.g. clears any repository write-through cache
    return Response::success(CommandType::ClearServerCache, "Cache cleared");
}

RestartServerCommand::RestartServerCommand(AdminService* adminService)
    : m_adminService(adminService)
{
}

Response RestartServerCommand::execute(const QVariantMap& params)
{
    // Returning success and scheduling the restart async is safer than
    // restarting inline, so the response actually reaches the admin
    // before the socket drops.
    m_adminService->scheduleRestart(); // new
    return Response::success(CommandType::RestartServer, "Restart scheduled");
}


GetServerResourceUsageCommand::GetServerResourceUsageCommand(ClientHandler* clientHandler)
    : m_clientHandler(clientHandler) {}

Response GetServerResourceUsageCommand::execute(const QVariantMap& params) {
    Q_UNUSED(params);
    if (!m_clientHandler) return Response::error(CommandType::GetServerResourceUsage, "Server unavailable");
    return Response::success(CommandType::GetServerResourceUsage, m_clientHandler->getServerResourceUsage());
}

GetConnectedClientsCommand::GetConnectedClientsCommand(ClientHandler* clientHandler)
    : m_clientHandler(clientHandler) {}

Response GetConnectedClientsCommand::execute(const QVariantMap& params) {
    Q_UNUSED(params);
    if (!m_clientHandler) return Response::error(CommandType::GetConnectedClients, "Server unavailable");
    QVariantList clients = m_clientHandler->getConnectedClientsInfo();
    QVariantMap data;
    data["clients"] = clients;
    data["count"] = clients.size();
    return Response::success(CommandType::GetConnectedClients, data);
}

GetTrafficStatsCommand::GetTrafficStatsCommand(ClientHandler* clientHandler)
    : m_clientHandler(clientHandler) {}

Response GetTrafficStatsCommand::execute(const QVariantMap& params) {
    Q_UNUSED(params);
    if (!m_clientHandler) return Response::error(CommandType::GetTrafficStats, "Server unavailable");
    return Response::success(CommandType::GetTrafficStats, m_clientHandler->getTrafficStats());
}


// Commands.cpp

// =============================================
// ===== GetAllBooksCommand =====
// =============================================

GetAllBooksCommand::GetAllBooksCommand(BookService* bookService)
    : m_bookService(bookService)
{
}

Response GetAllBooksCommand::execute(const QVariantMap& params)
{
    QString search = params.value("search").toString();
    int publisherId = params.value("publisherId", -1).toInt();
    QString status = params.value("status").toString();
    int limit = params.value("limit", 0).toInt();
    int offset = params.value("offset", 0).toInt();

    // 2. دریافت همه کتاب‌ها (فعال و غیرفعال)
    // برای ادمین همه کتاب‌ها را نشان می‌دهیم
    QVector<QSharedPointer<Book>> allBooks;

    if (!search.isEmpty()) {
        // جستجو با کلمه کلیدی
        allBooks = m_bookService->searchBooks(search);
    } else {
        // دریافت همه کتاب‌ها از Repository (نه فقط فعال)
        BookRepository* repo = m_bookService->getBookRepo();
        if (repo) {
            allBooks = repo->getAllBooks();
        } else {
            return Response::error(CommandType::GetAllBooks, "Failed to access book repository");
        }
    }

    // 3. اعمال فیلترها
    QVector<QSharedPointer<Book>> filteredBooks;

    for (const auto& book : allBooks) {
        // فیلتر بر اساس ناشر
        if (publisherId > 0 && book->getPublisherId() != publisherId) {
            continue;
        }

        // فیلتر بر اساس وضعیت
        if (status == "active" && !book->getIsActive()) {
            continue;
        }
        if (status == "inactive" && book->getIsActive()) {
            continue;
        }
        if (status == "flagged" && !book->getIsFlagged()) {
            continue;
        }

        filteredBooks.append(book);
    }

    // 4. مرتب‌سازی بر اساس ID (جدیدترین اول) - اختیاری
    std::sort(filteredBooks.begin(), filteredBooks.end(),
              [](const QSharedPointer<Book>& a, const QSharedPointer<Book>& b) {
                  return a->getBookId() > b->getBookId();
              });

    // 5. اعمال Pagination (اختیاری)
    int totalCount = filteredBooks.size();
    if (limit > 0) {
        int start = qMin(offset, totalCount);
        int end = qMin(start + limit, totalCount);
        filteredBooks = filteredBooks.mid(start, end - start);
    }

    // 6. ساخت لیست کتاب‌ها برای پاسخ
    QVariantList bookList;
    for (const auto& book : filteredBooks) {
        QVariantMap bookData;
        bookData["bookId"] = book->getBookId();
        bookData["title"] = book->getTitle();
        bookData["author"] = book->getAuthor();
        bookData["genre"] = GenreHelper::toString(book->getGenre());
        bookData["description"] = book->getDescription();
        bookData["price"] = book->getPrice();
        bookData["discountPercent"] = book->getDiscountPercent();
        bookData["finalPrice"] = book->getFinalPrice();
        bookData["averageRating"] = book->getAverageRating();
        bookData["salesCount"] = book->getSalesCount();
        bookData["coverPath"] = book->getCoverPath();
        bookData["pdfPath"] = book->getPdfPath();
        bookData["isActive"] = book->getIsActive();
        bookData["isFlagged"] = book->getIsFlagged();
        bookData["publisherId"] = book->getPublisherId();

        // اضافه کردن نام ناشر (اگر UserRepository در دسترس است)
        // Publisher* publisher = m_userService->getPublisherById(book->getPublisherId());
        // if (publisher) {
        //     bookData["publisherName"] = publisher->getPublisherName();
        // }

        bookList.append(bookData);
    }

    // 7. ساخت پاسخ
    QVariantMap data;
    data["books"] = bookList;
    data["count"] = bookList.size();
    data["totalCount"] = totalCount;

    return Response::success(CommandType::GetAllBooks, "Books loaded successfully", data);
}

static QVariantMap participantToMap(const SessionParticipant& p)
{
    QVariantMap pm;
    pm["userId"]      = p.userId;
    pm["username"]    = p.username;
    pm["role"]        = p.role;
    pm["currentPage"] = p.currentPage;
    pm["online"]      = p.online;
    // totalPages intentionally omitted: the client already knows it from
    // the bookData the window was opened with, and overrides this locally.
    return pm;
}

static QVariantList participantsToList(const ReadingSession& session)
{
    QVariantList list;
    for (const auto& p : session.participants) {
        list.append(participantToMap(p));
    }
    return list;
}



CreateReadingSessionCommand::CreateReadingSessionCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response CreateReadingSessionCommand::execute(const QVariantMap& params)
{
    int bookId = params.value("bookId").toInt();
    int userId = params.value("userId").toInt();
    QString username = params.value("username").toString();

    if (bookId <= 0 || userId <= 0 || username.isEmpty()) {
        return Response::error(CommandType::CreateReadingSession, "Missing required fields");
    }

    ReadingSession session = m_sessionService->createSession(bookId, userId, username);

    QVariantMap data;
    data["sessionId"]   = session.sessionId;
    data["sessionCode"] = session.sessionCode;
    return Response::success(CommandType::CreateReadingSession, "Session created", data);
}




JoinReadingSessionCommand::JoinReadingSessionCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response JoinReadingSessionCommand::execute(const QVariantMap& params)
{
    int userId = params.value("userId").toInt();
    QString username = params.value("username").toString();
    int sessionId = params.value("sessionId", -1).toInt();
    QString code = params.value("sessionCode").toString();

    if (userId <= 0 || username.isEmpty()) {
        return Response::error(CommandType::JoinReadingSession, "Missing user info");
    }
    if (sessionId <= 0 && !code.isEmpty()) {
        sessionId = m_sessionService->findSessionIdByCode(code);
    }
    if (sessionId <= 0) {
        return Response::error(CommandType::JoinReadingSession, "Session not found");
    }

    ReadingSession session;
    bool isNewParticipant = false;
    if (!m_sessionService->joinSession(sessionId, userId, username, session, isNewParticipant)) {
        return Response::error(CommandType::JoinReadingSession, "Failed to join session");
    }

    QVariantMap data;
    data["sessionId"]    = session.sessionId;
    data["sessionCode"]  = session.sessionCode;
    // Needed by clients who joined purely via invite code and never opened
    // this book locally - without it GroupReadingWindow has no way to know
    // which book's title/cover/PDF to load.
    data["bookId"]       = session.bookId;
    data["currentPage"]  = session.participants.value(session.hostUserId).currentPage;
    data["participants"] = participantsToList(session);

    QVariantList chatList;
    for (const auto& m : session.chatHistory) {
        QVariantMap cm;
        cm["senderId"]   = m.senderId;
        cm["senderName"] = m.senderName;
        cm["text"]       = m.text;
        cm["timestamp"]  = m.timestamp;
        cm["colorIndex"] = m.colorIndex;
        chatList.append(cm);
    }
    data["chatHistory"] = chatList;

    // Targeted push: tell every OTHER participant that this user joined, so
    // their participant list updates without waiting on the next poll tick.
    //
    // Fix: also push when the user is *rejoining* a session they were
    // already in (isNewParticipant == false). Previously this branch was
    // gated on isNewParticipant, so a reconnecting user's online flag just
    // flipped silently on the server and nobody else found out until the
    // next 3s sync. We send the same ParticipantUpdate push, but with a
    // "reconnected" flag set so the client can render a "X reconnected"
    // system message instead of (not in addition to) "X joined".
    if (m_handler) {
        QVariantMap notify;
        notify["userId"]      = userId;
        notify["username"]    = username;
        notify["role"]        = "Member";
        notify["currentPage"] = session.participants.value(userId).currentPage;
        notify["joined"]      = isNewParticipant;
        notify["reconnected"] = !isNewParticipant;

        const char* msg = isNewParticipant ? "Participant joined" : "Participant reconnected";
        Response push = Response::success(CommandType::ReadingSessionParticipantUpdate,
                                          msg, notify);
        for (int otherId : m_sessionService->otherParticipantIds(sessionId, userId)) {
            m_handler->sendToUser(otherId, push);
        }
    }

    return Response::success(CommandType::JoinReadingSession, "Joined session", data);
}

// ---------- LeaveReadingSessionCommand ----------

LeaveReadingSessionCommand::LeaveReadingSessionCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response LeaveReadingSessionCommand::execute(const QVariantMap& params)
{
    int sessionId = params.value("sessionId").toInt();
    int userId    = params.value("userId").toInt();

    if (sessionId <= 0 || userId <= 0) {
        return Response::error(CommandType::LeaveReadingSession, "Invalid session or user ID");
    }

    // Grab remaining participants BEFORE removal so we know who to notify.
    QVector<int> remaining = m_sessionService->otherParticipantIds(sessionId, userId);

    bool sessionEnded = false;
    int newHostUserId = -1;
    if (!m_sessionService->leaveSession(sessionId, userId, sessionEnded, newHostUserId)) {
        return Response::error(CommandType::LeaveReadingSession, "Session not found");
    }

    if (!sessionEnded && m_handler) {
        QVariantMap notify;
        notify["userId"] = userId;
        notify["joined"] = false;
        // Present only when the departing user was the host and someone
        // else was promoted - see ReadingSessionService::leaveSession().
        if (newHostUserId > 0) {
            notify["newHostId"] = newHostUserId;
        }
        Response push = Response::success(CommandType::ReadingSessionParticipantUpdate,
                                          "Participant left", notify);
        for (int otherId : remaining) {
            m_handler->sendToUser(otherId, push);
        }
    }

    QVariantMap data;
    data["sessionId"] = sessionId;
    return Response::success(CommandType::LeaveReadingSession, "Left session", data);
}

// ---------- ReadingSessionPageSyncCommand ----------

ReadingSessionPageSyncCommand::ReadingSessionPageSyncCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response ReadingSessionPageSyncCommand::execute(const QVariantMap& params)
{
    int sessionId = params.value("sessionId").toInt();
    int senderId  = params.value("senderId").toInt();
    int page      = params.value("page").toInt();

    if (!m_sessionService->updatePage(sessionId, senderId, page)) {
        return Response::error(CommandType::ReadingSessionPageSync, "Failed to sync page");
    }

    QVariantMap data;
    data["sessionId"] = sessionId;
    data["senderId"]  = senderId;
    data["page"]      = page;

    // Broadcast to every OTHER participant so their handleResponse() case
    // fires in near-real-time instead of waiting up to SYNC_INTERVAL_MS.
    if (m_handler) {
        Response push = Response::success(CommandType::ReadingSessionPageSync, "Page synced", data);
        for (int otherId : m_sessionService->otherParticipantIds(sessionId, senderId)) {
            m_handler->sendToUser(otherId, push);
        }
    }

    return Response::success(CommandType::ReadingSessionPageSync, "Page synced", data);
}

// ---------- ReadingSessionFullSyncCommand ----------

ReadingSessionFullSyncCommand::ReadingSessionFullSyncCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response ReadingSessionFullSyncCommand::execute(const QVariantMap& params)
{
    int sessionId = params.value("sessionId").toInt();
    int userId    = params.value("userId").toInt();
    int currentPage = params.value("currentPage", -1).toInt();

    ReadingSession session;
    if (!m_sessionService->getSession(sessionId, session)) {
        return Response::error(CommandType::ReadingSessionFullSync, "Session not found");
    }

    // Fix: the client sends its current page on every periodic sync (every
    // 3s) as a safety net in case a ReadingSessionPageSync broadcast was
    // ever dropped. Previously this param was silently ignored, so a
    // dropped page-sync broadcast would leave the server's view of this
    // user's page stale forever. Apply it here, then re-fetch the session
    // so the response we return reflects the just-updated page.
    if (userId > 0 && currentPage > 0) {
        m_sessionService->updatePage(sessionId, userId, currentPage);
        m_sessionService->getSession(sessionId, session);
    }

    QVariantMap data;
    data["participants"] = participantsToList(session);
    // chatHistory intentionally omitted from periodic sync - the client
    // only wants it once, from the initial Join/CreateReadingSession
    // response, so it doesn't wipe/replay the chat pane every tick.
    return Response::success(CommandType::ReadingSessionFullSync, "Synced", data);
}

// ---------- ReadingSessionChatCommand ----------

ReadingSessionChatCommand::ReadingSessionChatCommand(ReadingSessionService* sessionService, ClientHandler* handler)
    : m_sessionService(sessionService), m_handler(handler) {}

Response ReadingSessionChatCommand::execute(const QVariantMap& params)
{
    int sessionId = params.value("sessionId").toInt();

    SessionChatMessage msg;
    msg.senderId   = params.value("senderId").toInt();
    msg.senderName = params.value("senderName").toString();
    msg.text       = params.value("text").toString();
    msg.colorIndex = params.value("colorIndex").toInt();
    msg.timestamp  = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (!m_sessionService->appendChat(sessionId, msg)) {
        return Response::error(CommandType::ReadingSessionChat, "Session not found");
    }

    QVariantMap data;
    data["senderId"]   = msg.senderId;
    data["senderName"] = msg.senderName;
    data["text"]       = msg.text;
    data["colorIndex"] = msg.colorIndex;

    // Broadcast to every OTHER participant. The sender already added their
    // own message locally on send, and their handleResponse() drops any
    // echo of their own senderId, so no double-send back to them needed.
    if (m_handler) {
        Response push = Response::success(CommandType::ReadingSessionChat, "Message sent", data);
        for (int otherId : m_sessionService->otherParticipantIds(sessionId, msg.senderId)) {
            m_handler->sendToUser(otherId, push);
        }
    }

    return Response::success(CommandType::ReadingSessionChat, "Message sent", data);
}

// ---------- ReadingSessionParticipantUpdateCommand ----------
// Not directly client-invokable - it only ever originates server-side from
// Join/LeaveReadingSessionCommand via ClientHandler::sendToUser(). Present
// so CommandFactory has a well-defined (harmless) case if a client ever
// sends this type directly.

ReadingSessionParticipantUpdateCommand::ReadingSessionParticipantUpdateCommand(ClientHandler* handler)
    : m_handler(handler) {}

Response ReadingSessionParticipantUpdateCommand::execute(const QVariantMap& /*params*/)
{
    return Response::error(CommandType::ReadingSessionParticipantUpdate,
                           "This event is server-pushed only and cannot be invoked directly");
}









