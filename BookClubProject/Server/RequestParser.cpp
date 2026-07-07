// RequestParser.cpp
#include "RequestParser.h"
#include <QJsonDocument>
#include <QJsonObject>

RequestParser::RequestParser(QObject *parent)
    : QObject(parent)
{
}

Request RequestParser::parse(const QString& data)
{
    Request request;

    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        request.setValid(false);
        return request;
    }

    QJsonObject obj = doc.object();

    QString commandStr = obj["command"].toString();
    request.setCommandType(Request::stringToCommandType(commandStr));


    request.setParams(obj["params"].toObject().toVariantMap());
    request.setValid(true);

    return request;
}