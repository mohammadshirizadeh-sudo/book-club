// Command.h
#ifndef COMMANDS_H
#define COMMANDS_H

#include <QString>
#include <QVariantMap>
#include "Request.h"
#include "Response.h"
#include "../Repositories/ReviewRepository.h"
#include "../Services/LibraryService.h"
#include "../Services/AdminService.h"


// Forward declarations
class AuthService;
class UserService;
class BookService;
class CartService;
class PurchaseService;
class ReviewService;
class PublisherService;
class AdminService;
class ClientHandler;



class Command
{
public:
    virtual ~Command() = default;


    virtual Response execute(const QVariantMap& params) = 0;

    virtual QString getName() const = 0;
    virtual CommandType getType() const=0;


    virtual bool requiresAuth() const { return true; }


    virtual bool requiresAdmin() const { return false; }


    virtual bool requiresPublisher() const { return false; }
};




class RequestPasswordResetCommand : public Command
{
public:
    explicit RequestPasswordResetCommand(AuthService* authService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::RequestPasswordReset; }
    QString getName() const override { return "RequestPasswordReset"; }
    bool requiresAuth() const override { return false; }

private:
    AuthService* m_authService;
};

class ResetPasswordWithTokenCommand : public Command
{
public:
    explicit ResetPasswordWithTokenCommand(AuthService* authService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ResetPasswordWithToken; }
    QString getName() const override { return "ResetPasswordWithToken"; }
    bool requiresAuth() const override { return false; }

private:
    AuthService* m_authService;
};




class LoginCommand : public Command
{
public:
    LoginCommand(AuthService* authService, ClientHandler* clientHandler);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::Login; }
    QString getName() const override { return "Login"; }
    bool requiresAuth() const override { return false; }

private:
    AuthService* m_authService;
    ClientHandler* m_clientHandler;
};

class RegisterCommand : public Command
{
public:
    explicit RegisterCommand(AuthService* authService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::Register; }
    QString getName() const override { return "Register"; }
    bool requiresAuth() const override { return false; }

private:
    AuthService* m_authService;
};

class LogoutCommand : public Command
{
public:
    explicit LogoutCommand(AuthService* authService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::Logout; }
    QString getName() const override { return "Logout"; }

private:
    AuthService* m_authService;
};

class ResetPasswordCommand : public Command
{
public:
    explicit ResetPasswordCommand(AuthService* authService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ResetPassword; }
    QString getName() const override { return "ResetPassword"; }
    bool requiresAuth() const override { return false; }

private:
    AuthService* m_authService;
};

class ConfirmResetPasswordCommand : public Command
{
public:
    explicit ConfirmResetPasswordCommand(AuthService* authService);

    Response execute(const QVariantMap& params) override;

    CommandType getType() const override{return CommandType::ConfirmResetPassword;}

    QString getName() const override{return "ConfirmResetPassword";}

    bool requiresAuth() const override{return false;}

private:
    AuthService* m_authService;
};

// =============================================
// ===== User Commands =====
// =============================================

class GetProfileCommand : public Command
{
public:
    explicit GetProfileCommand(UserService* userService,PurchaseService* m_purchaseService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetProfile; }
    QString getName() const override { return "GetProfile"; }

private:
    UserService* m_userService;
    PurchaseService* m_purchaseService;
};

class UpdateProfileCommand : public Command
{
public:
    explicit UpdateProfileCommand(UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::UpdateProfile; }
    QString getName() const override { return "UpdateProfile"; }

private:
    UserService* m_userService;
};

class ChangePasswordCommand : public Command
{
public:
    explicit ChangePasswordCommand(UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ChangePassword; }
    QString getName() const override { return "ChangePassword"; }

private:
    UserService* m_userService;
};

class UpdateFavoriteGenresCommand : public Command
{
public:
    explicit UpdateFavoriteGenresCommand(UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::UpdateFavoriteGenres; }
    QString getName() const override { return "UpdateFavoriteGenres"; }

private:
    UserService* m_userService;
};

// =============================================
// ===== Book Commands =====
// =============================================

class SearchBooksCommand : public Command
{
public:
    explicit SearchBooksCommand(BookService* bookService  , UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::SearchBooks; }
    QString getName() const override { return "SearchBooks"; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};

class GetBookByIdCommand : public Command
{
public:
    explicit GetBookByIdCommand(BookService* bookService , UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetBookById; }
    QString getName() const override { return "GetBookById"; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};

class GetBooksByGenreCommand : public Command
{
public:
    explicit GetBooksByGenreCommand(BookService* bookService , UserService* userServie);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetBooksByGenre; }
    QString getName() const override { return "GetBooksByGenre"; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};

class GetAllGenresCommand : public Command
{
public:
    explicit GetAllGenresCommand();
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetAllGenres; }
    QString getName() const override { return "GetAllGenres"; }
    bool requiresAuth() const override { return false; }
};

class GetPopularBooksCommand : public Command
{
public:
    explicit GetPopularBooksCommand(BookService* bookService , UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetPopularBooks; }
    QString getName() const override { return "GetPopularBooks"; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};

class GetNewBooksCommand : public Command
{
public:
    explicit GetNewBooksCommand(BookService* bookService ,  UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetNewBooks; }
    QString getName() const override { return "GetNewBooks"; }

private:
    BookService* m_bookService;
     UserService* m_userService;
};

class GetFreeBooksCommand : public Command
{
public:
    explicit GetFreeBooksCommand(BookService* bookService , UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetFreeBooks; }
    QString getName() const override { return "GetFreeBooks"; }

private:
    BookService* m_bookService;
    UserService* m_userService;

};

class GetRecommendedBooksCommand : public Command
{
public:
    explicit GetRecommendedBooksCommand(BookService* bookService ,UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetRecommendedBooks; }
    QString getName() const override { return "GetRecommendedBooks"; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};

// =============================================
// ===== Cart Commands =====
// =============================================

class AddToCartCommand : public Command
{
public:
    explicit AddToCartCommand(CartService* cartService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::AddToCart; }
    QString getName() const override { return "AddToCart"; }

private:
    CartService* m_cartService;
};

class RemoveFromCartCommand : public Command
{
public:
    explicit RemoveFromCartCommand(CartService* cartService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::RemoveFromCart; }
    QString getName() const override { return "RemoveFromCart"; }

private:
    CartService* m_cartService;
};

class UpdateCartQuantityCommand : public Command
{
public:
    explicit UpdateCartQuantityCommand(CartService* cartService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::UpdateCartQuantity; }
    QString getName() const override { return "UpdateCartQuantity"; }

private:
    CartService* m_cartService;
};

class GetCartCommand : public Command
{
public:
    explicit GetCartCommand(CartService* cartService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetCart; }
    QString getName() const override { return "GetCart"; }

private:
    CartService* m_cartService;
};

class ClearCartCommand : public Command
{
public:
    explicit ClearCartCommand(CartService* cartService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ClearCart; }
    QString getName() const override { return "ClearCart"; }

private:
    CartService* m_cartService;
};

// =============================================
// ===== Purchase Commands =====
// =============================================

class CheckoutCommand : public Command
{
public:
    explicit CheckoutCommand(PurchaseService* purchaseService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::Checkout; }
    QString getName() const override { return "Checkout"; }

private:
    PurchaseService* m_purchaseService;
};

class GetPurchaseHistoryCommand : public Command
{
public:
    explicit GetPurchaseHistoryCommand(PurchaseService* purchaseService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetPurchaseHistory; }
    QString getName() const override { return "GetPurchaseHistory"; }

private:
    PurchaseService* m_purchaseService;
};

class GetPurchaseByIdCommand : public Command
{
public:
    explicit GetPurchaseByIdCommand(PurchaseService* purchaseService , BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetPurchaseById; }
    QString getName() const override { return "GetPurchaseById"; }

private:
    PurchaseService* m_purchaseService;
    BookService* m_bookService;
};

// =============================================
// ===== Review Commands =====
// =============================================

class AddReviewCommand : public Command
{
public:
    explicit AddReviewCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::AddReview; }
    QString getName() const override { return "AddReview"; }

private:
    ReviewService* m_reviewService;
};

class EditReviewCommand : public Command
{
public:
    explicit EditReviewCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::EditReview; }
    QString getName() const override { return "EditReview"; }

private:
    ReviewService* m_reviewService;
};

class DeleteReviewCommand : public Command
{
public:
    explicit DeleteReviewCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteReview; }
    QString getName() const override { return "DeleteReview"; }

private:
    ReviewService* m_reviewService;
};


class GetReviewsForBookCommand : public Command
{
public:
    explicit GetReviewsForBookCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetReviewsForBook; }
    QString getName() const override { return "GetReviewsForBook"; }

private:
    ReviewService* m_reviewService;
};

class GetAverageRatingCommand : public Command
{
public:
    explicit GetAverageRatingCommand(ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetAverageRating; }
    QString getName() const override { return "GetAverageRating"; }

private:
    ReviewService* m_reviewService;
};

// =============================================
// ===== Publisher Commands =====
// =============================================

class AddBookCommand : public Command
{
public:
    explicit AddBookCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::AddBook; }
    QString getName() const override { return "AddBook"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

class EditBookCommand : public Command
{
public:
    explicit EditBookCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::EditBook; }
    QString getName() const override { return "EditBook"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

class DeactivateBookCommand : public Command
{
public:
    explicit DeactivateBookCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeactivateBook; }
    QString getName() const override { return "DeactivateBook"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

class ReactivateBookCommand : public Command
{
public:
    explicit ReactivateBookCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ReactivateBook; }
    QString getName() const override { return "ReactivateBook"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

class GetPublisherBooksCommand : public Command
{
public:
    explicit GetPublisherBooksCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetPublisherBooks; }
    QString getName() const override { return "GetPublisherBooks"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

class GetPublisherStatsCommand : public Command
{
public:
    explicit GetPublisherStatsCommand(PublisherService* publisherService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetPublisherStats; }
    QString getName() const override { return "GetPublisherStats"; }
    bool requiresPublisher() const override { return true; }

private:
    PublisherService* m_publisherService;
};

// =============================================
// ===== Admin Commands =====
// =============================================

class BlockUserCommand : public Command
{
public:
    explicit BlockUserCommand(UserService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::BlockUser; }
    QString getName() const override { return "BlockUser"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;

};

class UnblockUserCommand : public Command
{
public:
    explicit UnblockUserCommand(UserService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::UnblockUser; }
    QString getName() const override { return "UnblockUser"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
};

class DeleteUserCommand : public Command
{
public:
    explicit DeleteUserCommand(UserService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteUser; }
    QString getName() const override { return "DeleteUser"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
};

class GetAllUsersCommand : public Command
{
public:
    explicit GetAllUsersCommand(UserService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetAllUsers; }
    QString getName() const override { return "GetAllUsers"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
};

class GetBlockedUsersCommand : public Command
{
public:
    explicit GetBlockedUsersCommand(UserService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetBlockedUsers; }
    QString getName() const override { return "GetBlockedUsers"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
};

class AdminDeleteBookCommand : public Command
{
public:
    explicit AdminDeleteBookCommand(UserService* adminService ,BookService* m_bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteBook; }
    QString getName() const override { return "AdminDeleteBook"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
    BookService* m_bookService;
};

class AdminDeleteReviewCommand : public Command
{
public:
    explicit AdminDeleteReviewCommand(UserService* adminService,ReviewService* reviewService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteReview; }
    QString getName() const override { return "AdminDeleteReview"; }
    bool requiresAdmin() const override { return true; }

private:
    UserService* m_adminService;
    ReviewService* m_reviewService;
};

class GetSystemStatsCommand : public Command
{
public:
    explicit GetSystemStatsCommand(AdminService* adminService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetSystemStats; }
    QString getName() const override { return "GetSystemStats"; }
    bool requiresAdmin() const override { return true; }

private:
    AdminService* m_adminService;
};




// Commands.h
class SearchUserCommand : public Command
{
public:
    explicit SearchUserCommand(UserService* userService , BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::SearchUsers; }
    QString getName() const override { return "SearchUsers"; }
    bool requiresAdmin() const override { return true; }  // فقط ادمین

private:
    UserService* m_userService;
    BookService* m_bookService;
};




// Commands.h
class SearchAuthorCommand : public Command
{
public:
    explicit SearchAuthorCommand(BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::SearchAuthors; }
    QString getName() const override { return "SearchAuthors"; }
    bool requiresAuth() const override { return true; }

private:
    BookService* m_bookService;
};



// Commands.h
class GetNotificationsCommand : public Command
{
public:
    explicit GetNotificationsCommand(NotificationService* notificationService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetNotifications; }
    QString getName() const override { return "GetNotifications"; }
    bool requiresAuth() const override { return true; }

private:
    NotificationService* m_notificationService;
};


// Commands.h
class MarkNotificationReadCommand : public Command
{
public:
    explicit MarkNotificationReadCommand(NotificationService* notificationService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::MarkNotificationRead; }
    QString getName() const override { return "MarkNotificationRead"; }
    bool requiresAuth() const override { return true; }

private:
    NotificationService* m_notificationService;
};




// Commands.h
class MarkAllNotificationsReadCommand : public Command
{
public:
    explicit MarkAllNotificationsReadCommand(NotificationService* notificationService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::MarkAllNotificationsRead; }
    QString getName() const override { return "MarkAllNotificationsRead"; }
    bool requiresAuth() const override { return true; }

private:
    NotificationService* m_notificationService;
};



class ClearAllNotificationsCommand : public Command
{
public:
    explicit ClearAllNotificationsCommand(NotificationService* notificationService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::ClearAllNotifications; }
    QString getName() const override { return "ClearAllNotifications"; }
    bool requiresAuth() const override { return true; }

private:
    NotificationService* m_notificationService;
};




// Commands.h
class GetUserShelvesCommand : public Command
{
public:
    explicit GetUserShelvesCommand(UserService* userService, LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetUserShelves; }
    QString getName() const override { return "GetUserShelves"; }
    bool requiresAuth() const override { return true; }

private:
    UserService* m_userService;
    LibraryService* m_libraryService;
};


class GetBooksInShelfCommand : public Command
{
public:
    explicit GetBooksInShelfCommand(LibraryService* libraryService, BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetBooksInShelf; }
    QString getName() const override { return "GetBooksInShelf"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
    BookService* m_bookService;
};


class CreateShelfCommand : public Command
{
public:
    explicit CreateShelfCommand(LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::CreateShelf; }
    QString getName() const override { return "CreateShelf"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
};







// Commands.h
class DeleteShelfCommand : public Command
{
public:
    explicit DeleteShelfCommand(LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::DeleteShelf; }
    QString getName() const override { return "DeleteShelf"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
};



class RenameShelfCommand : public Command
{
public:
    explicit RenameShelfCommand(LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::RenameShelf; }
    QString getName() const override { return "RenameShelf"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
};


// Commands.h
class RemoveBookFromShelfCommand : public Command
{
public:
    explicit RemoveBookFromShelfCommand(LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::RemoveBookFromShelf; }
    QString getName() const override { return "RemoveBookFromShelf"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
};


class MoveBookBetweenShelvesCommand : public Command
{
public:
    explicit MoveBookBetweenShelvesCommand(LibraryService* libraryService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::MoveBookBetweenShelves; }
    QString getName() const override { return "MoveBookBetweenShelves"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
};

class GetBestSellersCommand : public Command
{
public:
    explicit GetBestSellersCommand(BookService* bookService , UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetBestSellers; }
    QString getName() const override { return "GetBestSellers"; }
    bool requiresAuth() const override { return false; }

private:
    BookService* m_bookService;
    UserService* m_userService;
};



class GetBookCoverCommand : public Command
{
public:

    explicit GetBookCoverCommand(BookService* bookService);

    Response execute(
        const QVariantMap& params
        ) override;


    CommandType getType() const override { return CommandType::GetBookCover; }
    QString getName() const override { return "GetBookCoverCommand"; }
    bool requiresAuth() const override { return false; }


private:

    BookService* m_bookService;
};


class AddFavoriteBookCommand : public Command
{
public:
    explicit AddFavoriteBookCommand(UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::AddFavoriteBook; }
    QString getName() const override { return "AddFavoriteBook"; }
    bool requiresAuth() const override { return true; }

private:
    UserService* m_userService;
};

// Commands.h
class GetFavoriteBooksCommand : public Command
{
public:
    explicit GetFavoriteBooksCommand(UserService* userService, BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetFavoriteBooks; }
    QString getName() const override { return "GetFavoriteBooks"; }
    bool requiresAuth() const override { return true; }

private:
    UserService* m_userService;
    BookService* m_bookService;
};


// Commands.h
class RemoveFavoriteBookCommand : public Command
{
public:
    explicit RemoveFavoriteBookCommand(UserService* userService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::RemoveFavoriteBook; }
    QString getName() const override { return "RemoveFavoriteBook"; }
    bool requiresAuth() const override { return true; }

private:
    UserService* m_userService;
};


// Commands.h
class GetUserLibraryCommand : public Command
{
public:
    explicit GetUserLibraryCommand(LibraryService* libraryService, BookService* bookService);
    Response execute(const QVariantMap& params) override;
    CommandType getType() const override { return CommandType::GetUserLibrary; }
    QString getName() const override { return "GetUserLibrary"; }
    bool requiresAuth() const override { return true; }

private:
    LibraryService* m_libraryService;
    BookService* m_bookService;
};








#endif // COMMANDS_H
