// AccessLog.h
#ifndef ACCESSLOG_H
#define ACCESSLOG_H

#include <QString>
#include <QDateTime>

struct AccessLogEntry
{
    QDateTime timestamp;
    QString adminName;
    QString action;
    QString targetUser;
    QString ipAddress;
    QString status;

    AccessLogEntry() = default;

    AccessLogEntry(const QDateTime& time,
                   const QString& admin,
                   const QString& act,
                   const QString& target,
                   const QString& ip,
                   const QString& stat)
        : timestamp(time)
        , adminName(admin)
        , action(act)
        , targetUser(target)
        , ipAddress(ip)
        , status(stat)
    {
    }
};

#endif // ACCESSLOG_H