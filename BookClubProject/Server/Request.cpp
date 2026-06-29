// Request.cpp
#include "Request.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

// ===== Constructors =====

Request::Request()
    : m_command(Command::Unknown)
    , m_valid(false)
{
}

Request::Request(Command command, const QVariantMap& params)
    : m_command(command)
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

// ===== Command String Conversion =====

QString Request::getCommandString() const
{
    return commandToString(m_command);
}

Command Request::stringToCommand(const QString& str)
{
    static QMap<QString, Command> commandMap = {
        // Auth
        {"Login", Command::Login},
        {"Register", Command::Register},
        {"Logout", Command::Logout},
        {"ResetPassword", Command::ResetPassword},

        // User
        {"GetProfile", Command::GetProfile},
        {"UpdateProfile", Command::UpdateProfile},
        {"ChangePassword", Command::ChangePassword},
        {"UpdateFavoriteGenres", Command::UpdateFavoriteGenres},

        // Book
        {"SearchBooks", Command::SearchBooks},
        {"GetBookById", Command::GetBookById},
        {"GetBooksByGenre", Command::GetBooksByGenre},
        {"GetPopularBooks", Command::GetPopularBooks},
        {"GetNewBooks", Command::GetNewBooks},
        {"GetFreeBooks", Command::GetFreeBooks},
        {"GetRecommendedBooks", Command::GetRecommendedBooks},

        // Cart
        {"AddToCart", Command::AddToCart},
        {"RemoveFromCart", Command::RemoveFromCart},
        {"UpdateCartQuantity", Command::UpdateCartQuantity},
        {"GetCart", Command::GetCart},
        {"ClearCart", Command::ClearCart},

        // Purchase
        {"Checkout", Command::Checkout},
        {"GetPurchaseHistory", Command::GetPurchaseHistory},
        {"GetPurchaseById", Command::GetPurchaseById},

        // Review
        {"AddReview", Command::AddReview},
        {"EditReview", Command::EditReview},
        {"DeleteReview", Command::DeleteReview},
        {"GetReviewsForBook", Command::GetReviewsForBook},
        {"GetAverageRating", Command::GetAverageRating},

        // Publisher
        {"AddBook", Command::AddBook},
        {"EditBook", Command::EditBook},
        {"DeactivateBook", Command::DeactivateBook},
        {"ReactivateBook", Command::ReactivateBook},
        {"GetPublisherBooks", Command::GetPublisherBooks},
        {"GetPublisherStats", Command::GetPublisherStats},

        // Admin
        {"BlockUser", Command::BlockUser},
        {"UnblockUser", Command::UnblockUser},
        {"DeleteUser", Command::DeleteUser},
        {"GetAllUsers", Command::GetAllUsers},
        {"GetBlockedUsers", Command::GetBlockedUsers},
        {"DeleteBook", Command::DeleteBook},
        {"DeleteReview", Command::DeleteReview},
        {"GetSystemStats", Command::GetSystemStats}
    };

    return commandMap.value(str, Command::Unknown);
}

QString Request::commandToString(Command cmd)
{
    switch(cmd) {
    // Auth
    case Command::Login: return "Login";
    case Command::Register: return "Register";
    case Command::Logout: return "Logout";
    case Command::ResetPassword: return "ResetPassword";

    // User
    case Command::GetProfile: return "GetProfile";
    case Command::UpdateProfile: return "UpdateProfile";
    case Command::ChangePassword: return "ChangePassword";
    case Command::UpdateFavoriteGenres: return "UpdateFavoriteGenres";

    // Book
    case Command::SearchBooks: return "SearchBooks";
    case Command::GetBookById: return "GetBookById";
    case Command::GetBooksByGenre: return "GetBooksByGenre";
    case Command::GetPopularBooks: return "GetPopularBooks";
    case Command::GetNewBooks: return "GetNewBooks";
    case Command::GetFreeBooks: return "GetFreeBooks";
    case Command::GetRecommendedBooks: return "GetRecommendedBooks";

    // Cart
    case Command::AddToCart: return "AddToCart";
    case Command::RemoveFromCart: return "RemoveFromCart";
    case Command::UpdateCartQuantity: return "UpdateCartQuantity";
    case Command::GetCart: return "GetCart";
    case Command::ClearCart: return "ClearCart";

    // Purchase
    case Command::Checkout: return "Checkout";
    case Command::GetPurchaseHistory: return "GetPurchaseHistory";
    case Command::GetPurchaseById: return "GetPurchaseById";

    // Review
    case Command::AddReview: return "AddReview";
    case Command::EditReview: return "EditReview";
    case Command::DeleteReview: return "DeleteReview";
    case Command::GetReviewsForBook: return "GetReviewsForBook";
    case Command::GetAverageRating: return "GetAverageRating";

    // Publisher
    case Command::AddBook: return "AddBook";
    case Command::EditBook: return "EditBook";
    case Command::DeactivateBook: return "DeactivateBook";
    case Command::ReactivateBook: return "ReactivateBook";
    case Command::GetPublisherBooks: return "GetPublisherBooks";
    case Command::GetPublisherStats: return "GetPublisherStats";

    // Admin
    case Command::BlockUser: return "BlockUser";
    case Command::UnblockUser: return "UnblockUser";
    case Command::DeleteUser: return "DeleteUser";
    case Command::GetAllUsers: return "GetAllUsers";
    case Command::GetBlockedUsers: return "GetBlockedUsers";
    case Command::DeleteBook: return "DeleteBook";
    case Command::GetSystemStats: return "GetSystemStats";

    default: return "Unknown";
    }
}

// ===== Conversion =====

QJsonObject Request::toJson() const
{
    QJsonObject obj;
    obj["command"] = getCommandString();

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

    QString commandStr = json["command"].toString();
    request.setCommand(stringToCommand(commandStr));

    if (json.contains("params") && json["params"].isObject()) {
        QJsonObject paramsObj = json["params"].toObject();
        QVariantMap paramsMap;
        for (auto it = paramsObj.begin(); it != paramsObj.end(); ++it) {
            paramsMap[it.key()] = it.value().toVariant();
        }
        request.setParams(paramsMap);
    }

    request.setValid(request.getCommand() != Command::Unknown);
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