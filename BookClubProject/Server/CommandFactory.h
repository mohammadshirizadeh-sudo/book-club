// CommandFactory.h
#ifndef COMMANDFACTORY_H
#define COMMANDFACTORY_H

#include"Commands.h"

// Forward declarations
class AuthService;
class BookService;
class UserService;
class PurchaseService;
class ReviewService;
class CartService;
class PublisherService;
class AdminService;

/**
 * @brief Factory class for creating Command objects
 *
 * This class is responsible for creating the appropriate Command
 * based on the CommandType enum value.
 */
class CommandFactory
{
public:

    static Command* create(
        CommandType type,
        AuthService* authService,
        BookService* bookService,
        UserService* userService,
        PurchaseService* purchaseService,
        ReviewService* reviewService,
        CartService* cartService,
        PublisherService* publisherService,
        AdminService* adminService,ClientHandler* clientHandler
        );

private:
    // Private constructor to prevent instantiation (all methods are static)
    CommandFactory() = delete;
    ~CommandFactory() = delete;
};

#endif // COMMANDFACTORY_H