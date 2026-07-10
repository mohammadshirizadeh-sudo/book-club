// Response.cpp
#include "Response.h"
#include <QJsonDocument>
#include <QDebug>

Response::Response()
    : m_success(false)
    , m_message("")
    , m_errorCode(0)
{
}

Response::Response(bool success, const QString& message, const QVariantMap& data)
    : m_success(success)
    , m_message(message)
    , m_data(data)
    , m_errorCode(0)
{
}

// ===== Static factory methods =====

Response Response::success(const QString& message)
{
    return Response(true, message, QVariantMap());
}

Response Response::success(const QVariantMap& data)
{
    return Response(true, "", data);
}

Response Response::success(const QString& message, const QVariantMap& data)
{
    return Response(true, message, data);
}

Response Response::error(const QString& message)
{
    Response resp(false, message, QVariantMap());
    resp.setErrorCode(400);
    return resp;
}

Response Response::error(int code, const QString& message)
{
    Response resp(false, message, QVariantMap());
    resp.setErrorCode(code);
    return resp;
}

// ===== Conversion =====

QJsonObject Response::toJson() const
{
    QJsonObject obj;
    obj["success"] = m_success;
    obj["message"] = m_message;
    obj["errorCode"] = m_errorCode;

    // Convert QVariantMap to QJsonObject
    if (!m_data.isEmpty()) {
        QJsonObject dataObj;
        for (auto it = m_data.begin(); it != m_data.end(); ++it) {
            dataObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj["data"] = dataObj;
    }

    return obj;
}

QString Response::toJsonString() const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QByteArray Response::toUtf8() const
{
    QJsonDocument doc(toJson());
    return doc.toJson(QJsonDocument::Compact);
}

Response Response::fromJson(const QJsonObject& json)
{
    Response resp;
    resp.setSuccess(json["success"].toBool(false));
    resp.setMessage(json["message"].toString(""));
    resp.setErrorCode(json["errorCode"].toInt(0));

    if (json.contains("data") && json["data"].isObject()) {
        QJsonObject dataObj = json["data"].toObject();
        QVariantMap dataMap;
        for (auto it = dataObj.begin(); it != dataObj.end(); ++it) {
            dataMap[it.key()] = it.value().toVariant();
        }
        resp.setData(dataMap);
    }

    return resp;
}