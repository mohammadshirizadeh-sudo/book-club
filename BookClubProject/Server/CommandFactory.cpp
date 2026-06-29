// CommandFactory.cpp
#include "CommandFactory.h"

#include "Commands.h"
#include <QDebug>

Command* CommandFactory::create(
    CommandType type,
    AuthService* authService,
    BookService* bookService,
    UserService* userService,
    PurchaseService* purchaseService,
    ReviewService* reviewService,
    CartService* cartService,
    PublisherService* publisherService,
    AdminService* adminService)
{
    switch (type) {
    // =============================================
    // ===== Auth Commands =====
    // =============================================
    case CommandType::Login:
        return new LoginCommand(authService);

    case CommandType::Register:
        return new RegisterCommand(authService);

    case CommandType::Logout:
        return new LogoutCommand(authService);

    case CommandType::ResetPassword:
        return new ResetPasswordCommand(authService);

    // =============================================
    // ===== User Commands =====
    // =============================================
    case CommandType::GetProfile:
        return new GetProfileCommand(userService);

    case CommandType::UpdateProfile:
        return new UpdateProfileCommand(userService);

    case CommandType::ChangePassword:
        return new ChangePasswordCommand(userService);

    case CommandType::UpdateFavoriteGenres:
        return new UpdateFavoriteGenresCommand(userService);

    // =============================================
    // ===== Book Commands =====
    // =============================================
    case CommandType::SearchBooks:
        return new SearchBooksCommand(bookService);

    case CommandType::GetBookById:
        return new GetBookByIdCommand(bookService);

    case CommandType::GetBooksByGenre:
        return new GetBooksByGenreCommand(bookService);

    case CommandType::GetPopularBooks:
        return new GetPopularBooksCommand(bookService);

    case CommandType::GetNewBooks:
        return new GetNewBooksCommand(bookService);

    case CommandType::GetFreeBooks:
        return new GetFreeBooksCommand(bookService);

    case CommandType::GetRecommendedBooks:
        return new GetRecommendedBooksCommand(bookService);

    // =============================================
    // ===== Cart Commands =====
    // =============================================
    case CommandType::AddToCart:
        return new AddToCartCommand(cartService);

    case CommandType::RemoveFromCart:
        return new RemoveFromCartCommand(cartService);

    case CommandType::UpdateCartQuantity:
        return new UpdateCartQuantityCommand(cartService);

    case CommandType::GetCart:
        return new GetCartCommand(cartService);

    case CommandType::ClearCart:
        return new ClearCartCommand(cartService);

    // =============================================
    // ===== Purchase Commands =====
    // =============================================
    case CommandType::Checkout:
        return new CheckoutCommand(purchaseService);

    case CommandType::GetPurchaseHistory:
        return new GetPurchaseHistoryCommand(purchaseService);

    case CommandType::GetPurchaseById:
        return new GetPurchaseByIdCommand(purchaseService);

    // =============================================
    // ===== Review Commands =====
    // =============================================
    case CommandType::AddReview:
        return new AddReviewCommand(reviewService);

    case CommandType::EditReview:
        return new EditReviewCommand(reviewService);

    case CommandType::DeleteReview:
        return new DeleteReviewCommand(reviewService);

    case CommandType::GetReviewsForBook:
        return new GetReviewsForBookCommand(reviewService);

    case CommandType::GetAverageRating:
        return new GetAverageRatingCommand(reviewService);

    // =============================================
    // ===== Publisher Commands =====
    // =============================================
    case CommandType::AddBook:
        return new AddBookCommand(publisherService);

    case CommandType::EditBook:
        return new EditBookCommand(publisherService);

    case CommandType::DeactivateBook:
        return new DeactivateBookCommand(publisherService);

    case CommandType::ReactivateBook:
        return new ReactivateBookCommand(publisherService);

    case CommandType::GetPublisherBooks:
        return new GetPublisherBooksCommand(publisherService);

    case CommandType::GetPublisherStats:
        return new GetPublisherStatsCommand(publisherService);

    // =============================================
    // ===== Admin Commands =====
    // =============================================
    case CommandType::BlockUser:
        return new BlockUserCommand(adminService);

    case CommandType::UnblockUser:
        return new UnblockUserCommand(adminService);

    case CommandType::DeleteUser:
        return new DeleteUserCommand(adminService);

    case CommandType::GetAllUsers:
        return new GetAllUsersCommand(adminService);

    case CommandType::GetBlockedUsers:
        return new GetBlockedUsersCommand(adminService);

    case CommandType::DeleteBook:
        return new AdminDeleteBookCommand(adminService);

    case CommandType::DeleteReview:
        return new AdminDeleteReviewCommand(adminService);

    case CommandType::GetSystemStats:
        return new GetSystemStatsCommand(adminService);

    // =============================================
    // ===== Unknown Command =====
    // =============================================
    default:
        qWarning() << "Unknown command type:" << static_cast<int>(type);
        return nullptr;
    }
}