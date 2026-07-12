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

Response::Response(CommandType type , bool success, const QString& message, const QVariantMap& data)
    : m_success(success)
    , m_message(message)
    , m_data(data)
    , m_errorCode(0)
    ,m_commandType(type)
{
}

// ===== Static factory methods =====

Response Response::success(CommandType type,const QString& message)
{
    return Response(type , true, message, QVariantMap());
}


Response Response::success(CommandType type,const QVariantMap& data)
{
    return Response(type ,true, "", data);
}

Response Response::success(CommandType type,const QString& message, const QVariantMap& data)
{
    return Response(type ,true, message, data);
}

Response Response::error(CommandType type,const QString& message)
{
    Response resp(type ,false, message, QVariantMap());
    resp.setErrorCode(400);
    return resp;
}

Response Response::error(CommandType type,int code, const QString& message)
{
    Response resp(type ,false, message, QVariantMap());
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
    obj["command"] = Request::CommandTypeToString(m_commandType);

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
    resp.setCommandType(Request::stringToCommandType(json["command"].toString()));

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