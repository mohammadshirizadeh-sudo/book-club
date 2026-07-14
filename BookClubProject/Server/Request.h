// Request.h
#ifndef REQUEST_H
#define REQUEST_H

#include <QString>
#include <QVariantMap>
#include <QJsonObject>


enum class CommandType {
    Unknown,            // Invalid/unknown command

    // ===== Auth =====
    Login,
    Register,
    Logout,
    ResetPassword,
    RequestPasswordReset,
    ResetPasswordWithToken,
    ConfirmResetPassword,

    // ===== User =====
    GetProfile,
    UpdateProfile,
    ChangePassword,
    UpdateFavoriteGenres,

    // ===== Book =====
    SearchBooks,
    GetBookById,
    GetBooksByGenre,
    GetPopularBooks,
    GetNewBooks,
    GetFreeBooks,
    GetRecommendedBooks,

    // ===== Cart =====
    AddToCart,
    RemoveFromCart,
    UpdateCartQuantity,
    GetCart,
    ClearCart,

    // ===== Purchase =====
    Checkout,
    GetPurchaseHistory,
    GetPurchaseById,

    // ===== Review =====
    AddReview,
    EditReview,
    DeleteReview,
    DeleteOwnReview,
    GetReviewsForBook,
    GetAverageRating,

    // ===== Publisher =====
    AddBook,
    EditBook,
    DeactivateBook,
    ReactivateBook,
    GetPublisherBooks,
    GetPublisherStats,

    // ===== Admin =====
    BlockUser,
    UnblockUser,
    DeleteUser,
    GetAllUsers,
    GetBlockedUsers,
    DeleteBook,

    GetSystemStats,




    SearchUsers,


    SearchAuthors
};


class Request
{
public:
    Request();
    Request(CommandType CommandType, const QVariantMap& params = QVariantMap());

    // ===== Getters =====
    CommandType getCommandType() const { return m_CommandType; }
    QString getCommandTypeString() const;
    QVariantMap getParams() const { return m_params; }
    bool isValid() const { return m_valid; }

    // ===== Setters =====
    void setCommandType(CommandType CommandType) { m_CommandType = CommandType; }
    void setParams(const QVariantMap& params) { m_params = params; }
    void setValid(bool valid) { m_valid = valid; }

    // ===== Param Helpers =====
    QVariant getParam(const QString& key) const;
    QString getParamString(const QString& key, const QString& defaultValue = "") const;
    int getParamInt(const QString& key, int defaultValue = 0) const;
    double getParamDouble(const QString& key, double defaultValue = 0.0) const;
    bool getParamBool(const QString& key, bool defaultValue = false) const;
    QStringList getParamStringList(const QString& key) const;
    QVariantMap getParamMap(const QString& key) const;

    // ===== Conversion =====
    QJsonObject toJson() const;
    QString toJsonString() const;
    QByteArray toUtf8() const;

    // ===== Static helpers =====
    static Request fromJson(const QJsonObject& json);
    static Request fromJsonString(const QString& jsonString);
    static CommandType stringToCommandType(const QString& str);
    static QString CommandTypeToString(CommandType cmd);

private:
    CommandType m_CommandType;
    QVariantMap m_params;
    bool m_valid;
};

#endif // REQUEST_H