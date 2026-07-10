// Response.h
#ifndef RESPONSE_H
#define RESPONSE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include "Request.h"

class Response
{
public:
    Response();
    Response(CommandType type , bool success, const QString& message = "", const QVariantMap& data = QVariantMap());

    static Response success(CommandType type, const QString& message = "");
    static Response success(CommandType type, const QVariantMap& data);
    static Response success(CommandType type, const QString& message, const QVariantMap& data);
    static Response error(CommandType type, const QString& message);
    static Response error(CommandType type, int code, const QString& message);

    // ===== Getters =====
    bool isSuccess() const { return m_success; }
    QString getMessage() const { return m_message; }
    QVariantMap getData() const { return m_data; }
    int getErrorCode() const { return m_errorCode; }
    CommandType getCommandType() const { return m_commandType; }

    // ===== Setters =====
    void setSuccess(bool success) { m_success = success; }
    void setMessage(const QString& message) { m_message = message; }
    void setData(const QVariantMap& data) { m_data = data; }
    void setErrorCode(int code) { m_errorCode = code; }
    void setCommandType(CommandType type) { m_commandType = type; }

    // ===== Conversion =====
    QJsonObject toJson() const;
    QString toJsonString() const;
    QByteArray toUtf8() const;

    // ===== Static helpers =====
    static Response fromJson(const QJsonObject& json);

private:
    bool m_success;
    QString m_message;
    QVariantMap m_data;
    int m_errorCode;
    CommandType m_commandType = CommandType::Unknown;
};
Q_DECLARE_METATYPE(Response)

#endif // RESPONSE_H