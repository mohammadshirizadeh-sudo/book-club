// AdminService.h
#include <QVector>
#include "AccessLog.h"

class AdminService : public QObject
{
    Q_OBJECT

public:

    void appendAccessLog(const AccessLogEntry& entry);

    QVector<AccessLogEntry> getAccessLogs() const;
    void clearAccessLogs();


private:
    QVector<AccessLogEntry> m_accessLogs;
    mutable QMutex m_logMutex;
};