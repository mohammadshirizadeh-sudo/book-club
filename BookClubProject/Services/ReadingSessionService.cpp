#include "ReadingSessionService.h"

ReadingSessionService::ReadingSessionService(QObject* parent) : QObject(parent) {}

QString ReadingSessionService::generateSessionCode() const
{
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"; // no O/0/I/1 confusion
    QString code;
    for (int i = 0; i < 6; ++i)
        code += chars.at(QRandomGenerator::global()->bounded(chars.length()));
    return code;
}

ReadingSession ReadingSessionService::createSession(int bookId, int hostUserId, const QString& hostUsername)
{
    QMutexLocker locker(&m_mutex);

    ReadingSession session;
    session.sessionId = m_nextSessionId++;
    session.sessionCode = generateSessionCode();
    session.bookId = bookId;
    session.hostUserId = hostUserId;

    SessionParticipant host;
    host.userId = hostUserId;
    host.username = hostUsername;
    host.role = "Host";
    host.currentPage = 1;
    host.online = true;
    session.participants[hostUserId] = host;

    m_sessions[session.sessionId] = session;
    return session;
}

bool ReadingSessionService::joinSession(int sessionId, int userId, const QString& username,
                                        ReadingSession& outSession, bool& isNewParticipant)
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(sessionId)) return false;

    ReadingSession& session = m_sessions[sessionId];
    isNewParticipant = !session.participants.contains(userId);

    if (isNewParticipant) {
        SessionParticipant p;
        p.userId = userId;
        p.username = username;
        p.role = "Member";
        p.currentPage = session.participants.value(session.hostUserId).currentPage;
        p.online = true;
        session.participants[userId] = p;
    } else {
        session.participants[userId].online = true;
    }

    outSession = session;
    return true;
}

bool ReadingSessionService::leaveSession(int sessionId, int userId, bool& sessionEnded, int& newHostUserId)
{
    QMutexLocker locker(&m_mutex);
    sessionEnded = false;
    newHostUserId = -1;
    if (!m_sessions.contains(sessionId)) return false;

    ReadingSession& session = m_sessions[sessionId];
    bool wasHost = (session.hostUserId == userId);
    session.participants.remove(userId);

    if (session.participants.isEmpty()) {
        m_sessions.remove(sessionId);
        sessionEnded = true;
        return true;
    }

    if (wasHost) {
        // The host left but the session is still populated - promote
        // someone else so "follow the host" (client-side) and the
        // starting-page lookup in JoinReadingSessionCommand (which reads
        // session.hostUserId) keep working. QMap iteration order is by
        // key (userId), not join order, so this is a deterministic but
        // arbitrary pick - any consistent choice is fine here, since the
        // only requirement is that *someone* is always the host.
        auto it = session.participants.begin();
        it.value().role = "Host";
        session.hostUserId = it.key();
        newHostUserId = it.key();
    }

    return true;
}

bool ReadingSessionService::updatePage(int sessionId, int userId, int page)
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(sessionId)) return false;
    if (!m_sessions[sessionId].participants.contains(userId)) return false;
    m_sessions[sessionId].participants[userId].currentPage = page;
    return true;
}

bool ReadingSessionService::appendChat(int sessionId, const SessionChatMessage& msg)
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(sessionId)) return false;
    m_sessions[sessionId].chatHistory.append(msg);
    return true;
}

bool ReadingSessionService::getSession(int sessionId, ReadingSession& outSession) const
{
    QMutexLocker locker(&m_mutex);
    if (!m_sessions.contains(sessionId)) return false;
    outSession = m_sessions[sessionId];
    return true;
}

int ReadingSessionService::findSessionIdByCode(const QString& code) const
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_sessions.constBegin(); it != m_sessions.constEnd(); ++it) {
        if (it.value().sessionCode.compare(code, Qt::CaseInsensitive) == 0)
            return it.key();
    }
    return -1;
}

QVector<int> ReadingSessionService::otherParticipantIds(int sessionId, int excludeUserId) const
{
    QMutexLocker locker(&m_mutex);
    QVector<int> ids;
    if (!m_sessions.contains(sessionId)) return ids;
    for (auto it = m_sessions[sessionId].participants.constBegin();
         it != m_sessions[sessionId].participants.constEnd(); ++it) {
        if (it.key() != excludeUserId) ids.append(it.key());
    }
    return ids;
}