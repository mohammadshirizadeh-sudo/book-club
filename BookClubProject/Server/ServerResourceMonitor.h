#ifndef SERVERRESOURCEMONITOR_H
#define SERVERRESOURCEMONITOR_H

#include <QtGlobal>

struct ResourceUsage {
    bool available    = false;  // false => platform unsupported, UI shows N/A
    double cpuPercent = 0.0;    // CPU usage percentage of total capacity
    qint64 ramUsedKB  = 0;      // Resident memory of this process
    qint64 ramTotalKB = 0;      // Total system RAM
    double ramPercent = 0.0;    // ramUsedKB / ramTotalKB * 100
};

class ServerResourceMonitor {
public:
    ServerResourceMonitor();
    ResourceUsage sample();

private:
    qint64 m_lastProcessTicks = -1;
    qint64 m_lastSystemTicks  = -1;

#if defined(Q_OS_LINUX)
    bool readLinuxCpuTicks(qint64 &processTicks, qint64 &systemTicks) const;
    qint64 readLinuxRamUsedKB() const;
    qint64 readLinuxRamTotalKB() const;
#elif defined(Q_OS_WIN)
    void *m_processHandle = nullptr;
#endif
};

#endif // SERVERRESOURCEMONITOR_H