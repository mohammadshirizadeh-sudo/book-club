// Response.h
#ifndef RESPONSE_H
#define RESPONSE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>

class Response
{
public:
    Response();
    Response(bool success, const QString& message = "", const QVariantMap& data = QVariantMap());

    // ===== Static factory methods =====
    static Response success(const QString& message = "");
    static Response success(const QVariantMap& data);
    static Response success(const QString& message, const QVariantMap& data);
    static Response error(const QString& message);
    static Response error(int code, const QString& message);

    // ===== Getters =====
    bool isSuccess() const { return m_success; }
    QString getMessage() const { return m_message; }
    QVariantMap getData() const { return m_data; }
    int getErrorCode() const { return m_errorCode; }

    // ===== Setters =====
    void setSuccess(bool success) { m_success = success; }
    void setMessage(const QString& message) { m_message = message; }
    void setData(const QVariantMap& data) { m_data = data; }
    void setErrorCode(int code) { m_errorCode = code; }

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
};
Q_DECLARE_METATYPE(Response)

#endif // RESPONSE_H