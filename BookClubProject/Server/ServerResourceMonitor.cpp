#include "ServerResourceMonitor.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <psapi.h>
#endif

ServerResourceMonitor::ServerResourceMonitor() {
#if defined(Q_OS_WIN)
    m_processHandle = GetCurrentProcess();
#endif
}

#if defined(Q_OS_LINUX)
bool ServerResourceMonitor::readLinuxCpuTicks(qint64 &processTicks, qint64 &systemTicks) const {
    QFile selfStat("/proc/self/stat");
    if (!selfStat.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QString line = QString::fromUtf8(selfStat.readAll());
    int closeParen = line.lastIndexOf(')');
    if (closeParen < 0) return false;
    QStringList fields = line.mid(closeParen + 2).split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 15) return false;
    qint64 utime = fields[11].toLongLong();
    qint64 stime = fields[12].toLongLong();
    processTicks = utime + stime;

    QFile stat("/proc/stat");
    if (!stat.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QString cpuLine = QString::fromUtf8(stat.readLine());
    QStringList cpuFields = cpuLine.split(' ', Qt::SkipEmptyParts);
    if (cpuFields.isEmpty() || cpuFields[0] != "cpu") return false;
    qint64 total = 0;
    for (int i = 1; i < cpuFields.size(); ++i) total += cpuFields[i].toLongLong();
    systemTicks = total;
    return true;
}

qint64 ServerResourceMonitor::readLinuxRamUsedKB() const {
    QFile status("/proc/self/status");
    if (!status.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
    while (!status.atEnd()) {
        QByteArray line = status.readLine();
        if (line.startsWith("VmRSS:")) {
            QStringList parts = QString::fromUtf8(line).split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) return parts[1].toLongLong();
        }
    }
    return 0;
}

qint64 ServerResourceMonitor::readLinuxRamTotalKB() const {
    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly | QIODevice::Text)) return 0;
    QString line = QString::fromUtf8(meminfo.readLine());
    QStringList parts = line.split(' ', Qt::SkipEmptyParts);
    if (parts.size() >= 2) return parts[1].toLongLong();
    return 0;
}

ResourceUsage ServerResourceMonitor::sample() {
    ResourceUsage usage;

    qint64 processTicks = 0, systemTicks = 0;
    if (!readLinuxCpuTicks(processTicks, systemTicks)) return usage;

    if (m_lastProcessTicks >= 0 && m_lastSystemTicks >= 0) {
        qint64 processDelta = processTicks - m_lastProcessTicks;
        qint64 systemDelta  = systemTicks  - m_lastSystemTicks;
        if (systemDelta > 0) {
            usage.cpuPercent = (double(processDelta) / double(systemDelta)) * 100.0;
        }
    }
    m_lastProcessTicks = processTicks;
    m_lastSystemTicks  = systemTicks;

    usage.ramUsedKB  = readLinuxRamUsedKB();
    usage.ramTotalKB = readLinuxRamTotalKB();
    usage.ramPercent = usage.ramTotalKB > 0
                           ? (double(usage.ramUsedKB) / double(usage.ramTotalKB)) * 100.0
                           : 0.0;
    usage.available = true;
    return usage;
}

#elif defined(Q_OS_WIN)
ResourceUsage ServerResourceMonitor::sample() {
    ResourceUsage usage;
    HANDLE proc = static_cast<HANDLE>(m_processHandle);

    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(proc, &createTime, &exitTime, &kernelTime, &userTime))
        return usage;

    auto toTicks = [](const FILETIME &ft) -> qint64 {
        return (qint64(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };
    qint64 processTicks = toTicks(kernelTime) + toTicks(userTime);

    FILETIME idleTime, sysKernel, sysUser;
    if (!GetSystemTimes(&idleTime, &sysKernel, &sysUser)) return usage;
    qint64 systemTicks = toTicks(sysKernel) + toTicks(sysUser);

    if (m_lastProcessTicks >= 0 && m_lastSystemTicks >= 0) {
        qint64 processDelta = processTicks - m_lastProcessTicks;
        qint64 systemDelta  = systemTicks  - m_lastSystemTicks;
        if (systemDelta > 0)
            usage.cpuPercent = (double(processDelta) / double(systemDelta)) * 100.0;
    }
    m_lastProcessTicks = processTicks;
    m_lastSystemTicks  = systemTicks;

    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(proc, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        usage.ramUsedKB = static_cast<qint64>(pmc.WorkingSetSize) / 1024;
    }

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        usage.ramTotalKB = static_cast<qint64>(memStatus.ullTotalPhys) / 1024;
    }

    usage.ramPercent = usage.ramTotalKB > 0
                           ? (double(usage.ramUsedKB) / double(usage.ramTotalKB)) * 100.0
                           : 0.0;
    usage.available = true;
    return usage;
}

#else
ResourceUsage ServerResourceMonitor::sample() {
    return ResourceUsage();
}
#endif