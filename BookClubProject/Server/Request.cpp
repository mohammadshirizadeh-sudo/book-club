// Request.cpp
#include "Request.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

// ===== Constructors =====

Request::Request()
    : m_CommandType(CommandType::Unknown)
    , m_valid(false)
{
}

Request::Request(CommandType CommandType, const QVariantMap& params)
    : m_CommandType(CommandType)
    , m_params(params)
    , m_valid(true)
{
}

// ===== Param Helpers =====

QVariant Request::getParam(const QString& key) const
{
    return m_params.value(key);
}

QString Request::getParamString(const QString& key, const QString& defaultValue) const
{
    return m_params.value(key, defaultValue).toString();
}

int Request::getParamInt(const QString& key, int defaultValue) const
{
    return m_params.value(key, defaultValue).toInt();
}

double Request::getParamDouble(const QString& key, double defaultValue) const
{
    return m_params.value(key, defaultValue).toDouble();
}

bool Request::getParamBool(const QString& key, bool defaultValue) const
{
    return m_params.value(key, defaultValue).toBool();
}

QStringList Request::getParamStringList(const QString& key) const
{
    QStringList result;
    QVariant value = m_params.value(key);

    if (value.canConvert<QStringList>()) {
        return value.toStringList();
    }

    if (value.canConvert<QVariantList>()) {
        QVariantList list = value.toList();
        for (const QVariant& item : list) {
            result.append(item.toString());
        }
    }

    return result;
}

QVariantMap Request::getParamMap(const QString& key) const
{
    return m_params.value(key).toMap();
}

// ===== CommandType String Conversion =====

QString Request::getCommandTypeString() const
{
    return CommandTypeToString(m_CommandType);
}

CommandType Request::stringToCommandType(const QString& str)
{
    static QMap<QString, CommandType> CommandTypeMap = {
        // Auth
        {"Login", CommandType::Login},
        {"Register", CommandType::Register},
        {"Logout", CommandType::Logout},
        {"ResetPassword", CommandType::ResetPassword},
        {"ConfirmResetPassword", CommandType::ConfirmResetPassword},


        // User
        {"GetProfile", CommandType::GetProfile},
        {"UpdateProfile", CommandType::UpdateProfile},
        {"ChangePassword", CommandType::ChangePassword},
        {"UpdateFavoriteGenres", CommandType::UpdateFavoriteGenres},

        // Book
        {"SearchBooks", CommandType::SearchBooks},
        {"GetBookById", CommandType::GetBookById},
        {"GetBooksByGenre", CommandType::GetBooksByGenre},
        {"GetPopularBooks", CommandType::GetPopularBooks},
        {"GetNewBooks", CommandType::GetNewBooks},
        {"GetFreeBooks", CommandType::GetFreeBooks},
        {"GetRecommendedBooks", CommandType::GetRecommendedBooks},

        // Cart
        {"AddToCart", CommandType::AddToCart},
        {"RemoveFromCart", CommandType::RemoveFromCart},
        {"UpdateCartQuantity", CommandType::UpdateCartQuantity},
        {"GetCart", CommandType::GetCart},
        {"ClearCart", CommandType::ClearCart},

        // Purchase
        {"Checkout", CommandType::Checkout},
        {"GetPurchaseHistory", CommandType::GetPurchaseHistory},
        {"GetPurchaseById", CommandType::GetPurchaseById},

        // Review
        {"AddReview", CommandType::AddReview},
        {"EditReview", CommandType::EditReview},
        {"DeleteOwnReview", CommandType::DeleteOwnReview},
        {"GetReviewsForBook", CommandType::GetReviewsForBook},
        {"GetAverageRating", CommandType::GetAverageRating},

        // Publisher
        {"AddBook", CommandType::AddBook},
        {"EditBook", CommandType::EditBook},
        {"DeactivateBook", CommandType::DeactivateBook},
        {"ReactivateBook", CommandType::ReactivateBook},
        {"GetPublisherBooks", CommandType::GetPublisherBooks},
        {"GetPublisherStats", CommandType::GetPublisherStats},

        // Admin
        {"BlockUser", CommandType::BlockUser},
        {"UnblockUser", CommandType::UnblockUser},
        {"DeleteUser", CommandType::DeleteUser},
        {"GetAllUsers", CommandType::GetAllUsers},
        {"GetBlockedUsers", CommandType::GetBlockedUsers},
        {"DeleteBook", CommandType::DeleteBook},
        {"DeleteReview", CommandType::DeleteReview},
        {"GetSystemStats", CommandType::GetSystemStats},



        {"RequestPasswordReset", CommandType::RequestPasswordReset},
        {"ResetPasswordWithToken", CommandType::ResetPasswordWithToken},


        {"SearchUsers", CommandType::SearchUsers},
        {"GetNotifications" , CommandType ::GetNotifications},
        {"MarkNotificationRead" , CommandType ::MarkNotificationRead},
        {"GetBooksInShelf" , CommandType ::GetBooksInShelf},

        {"GetUserShelves" , CommandType ::GetUserShelves},

        {"CreateShelf" , CommandType ::CreateShelf},
        {"DeleteShelf" , CommandType ::DeleteShelf},
        {"RenameShelf" , CommandType ::RenameShelf},

        {"RemoveBookFromShelf" , CommandType ::RemoveBookFromShelf},


        {"MoveBookBetweenShelves" , CommandType ::MoveBookBetweenShelves},

         {"GetBestSellers" , CommandType ::GetBestSellers},

        {"GetBookCover" , CommandType ::GetBookCover},


         {"AddFavoriteBook" , CommandType ::AddFavoriteBook},


        {"GetFavoriteBooks" , CommandType ::GetFavoriteBooks},
        {"RemoveFavoriteBook" , CommandType ::RemoveFavoriteBook},
        {"GetAllGenres" , CommandType ::GetAllGenres},
        {"GetUserLibrary" , CommandType ::GetUserLibrary},
        {"AddBookToShelf" , CommandType ::AddBookToShelf},
        {"GetBooksInShelf" , CommandType ::GetBooksInShelf},





    };

    return CommandTypeMap.value(str, CommandType::Unknown);
}

QString Request::CommandTypeToString(CommandType cmd)
{
    switch(cmd) {
    // Auth
    case CommandType::Login: return "Login";
    case CommandType::Register: return "Register";
    case CommandType::Logout: return "Logout";
    case CommandType::ResetPassword: return "ResetPassword";
    case CommandType::ConfirmResetPassword: return "ConfirmResetPassword";





    // User
    case CommandType::GetProfile: return "GetProfile";
    case CommandType::UpdateProfile: return "UpdateProfile";
    case CommandType::ChangePassword: return "ChangePassword";
    case CommandType::UpdateFavoriteGenres: return "UpdateFavoriteGenres";

    // Book
    case CommandType::SearchBooks: return "SearchBooks";
    case CommandType::GetBookById: return "GetBookById";
    case CommandType::GetBooksByGenre: return "GetBooksByGenre";
    case CommandType::GetPopularBooks: return "GetPopularBooks";
    case CommandType::GetNewBooks: return "GetNewBooks";
    case CommandType::GetFreeBooks: return "GetFreeBooks";
    case CommandType::GetRecommendedBooks: return "GetRecommendedBooks";

    // Cart
    case CommandType::AddToCart: return "AddToCart";
    case CommandType::RemoveFromCart: return "RemoveFromCart";
    case CommandType::UpdateCartQuantity: return "UpdateCartQuantity";
    case CommandType::GetCart: return "GetCart";
    case CommandType::ClearCart: return "ClearCart";

    // Purchase
    case CommandType::Checkout: return "Checkout";
    case CommandType::GetPurchaseHistory: return "GetPurchaseHistory";
    case CommandType::GetPurchaseById: return "GetPurchaseById";

    // Review
    case CommandType::AddReview: return "AddReview";
    case CommandType::EditReview: return "EditReview";
    case CommandType::DeleteReview: return "DeleteReview";
    case CommandType::GetReviewsForBook: return "GetReviewsForBook";
    case CommandType::GetAverageRating: return "GetAverageRating";

    // Publisher
    case CommandType::AddBook: return "AddBook";
    case CommandType::EditBook: return "EditBook";
    case CommandType::DeactivateBook: return "DeactivateBook";
    case CommandType::ReactivateBook: return "ReactivateBook";
    case CommandType::GetPublisherBooks: return "GetPublisherBooks";
    case CommandType::GetPublisherStats: return "GetPublisherStats";

    // Admin
    case CommandType::BlockUser: return "BlockUser";
    case CommandType::UnblockUser: return "UnblockUser";
    case CommandType::DeleteUser: return "DeleteUser";
    case CommandType::GetAllUsers: return "GetAllUsers";
    case CommandType::GetBlockedUsers: return "GetBlockedUsers";
    case CommandType::DeleteBook: return "DeleteBook";
    case CommandType::GetSystemStats: return "GetSystemStats";
    case CommandType::RequestPasswordReset: return "RequestPasswordReset";
    case CommandType::ResetPasswordWithToken: return "ResetPasswordWithToken";
    case CommandType::DeleteOwnReview: return "DeleteOwnReview";
    case CommandType::SearchUsers: return "SearchUsers";
    case CommandType::SearchAuthors : return "SearchAuthors";
    case CommandType::GetNotifications: return "GetNotifications";
    case CommandType::MarkAllNotificationsRead: return "MarkAllNotificationsRead";

    case CommandType::ClearAllNotifications: return "ClearAllNotifications";
    case CommandType::GetUserShelves: return "GetUserShelves";
    case CommandType::DeleteShelf: return "DeleteShelf";
    case CommandType::RenameShelf: return "RenameShelf";
    case CommandType::RemoveBookFromShelf: return "RemoveBookFromShelf";
    case CommandType::CreateShelf : return "CreateShelf";
    case CommandType::MoveBookBetweenShelves: return "MoveBookBetweenShelves";
    case CommandType::GetBooksInShelf: return "GetBooksInShelf";




    case CommandType::GetBestSellers: return "GetBestSellers";


    case CommandType::GetBookCover: return "GetBookCover";

    case CommandType::AddFavoriteBook: return "AddFavoriteBook";
    case CommandType::GetFavoriteBooks: return "GetFavoriteBooks";
    case CommandType::RemoveFavoriteBook: return "RemoveFavoriteBook";

    case CommandType::GetAllGenres: return "GetAllGenres";
    case CommandType::GetUserLibrary: return "GetUserLibrary";
    case CommandType::AddBookToShelf: return "AddBookToShelf";
    default: return "Unknown";
    }
}

// ===== Conversion =====

QJsonObject Request::toJson() const
{
    QJsonObject obj;
    obj["command"] = getCommandTypeString();

    if (!m_params.isEmpty()) {
        QJsonObject paramsObj;
        for (auto it = m_params.begin(); it != m_params.end(); ++it) {
            paramsObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj["params"] = paramsObj;
    }

    return obj;
}

QString Request::toJsonString() const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QByteArray Request::toUtf8() const
{
    QJsonDocument doc(toJson());
    return doc.toJson(QJsonDocument::Compact);
}

Request Request::fromJson(const QJsonObject& json)
{
    Request request;

    QString commandstr = json["command"].toString();
    request.setCommandType(stringToCommandType(commandstr));

    if (json.contains("params") && json["params"].isObject()) {
        QJsonObject paramsObj = json["params"].toObject();
        QVariantMap paramsMap;
        for (auto it = paramsObj.begin(); it != paramsObj.end(); ++it) {
            paramsMap[it.key()] = it.value().toVariant();
        }
        request.setParams(paramsMap);
    }

    request.setValid(request.getCommandType() != CommandType::Unknown);
    return request;
}

Request Request::fromJsonString(const QString& jsonString)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject()) {
        return Request();  // Invalid request
    }
    return fromJson(doc.object());
}