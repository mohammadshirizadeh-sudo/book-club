// RequestParser.h
#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H

#include <QObject>
#include <QString>
#include "Request.h"

class RequestParser : public QObject
{
    Q_OBJECT

public:
    explicit RequestParser(QObject *parent = nullptr);

    Request parse(const QString& data);
};

#endif // REQUESTPARSER_H