#ifndef READINGSESSIONSERVICE_H
#define READINGSESSIONSERVICE_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QMutex>
#include <QDateTime>
#include <QRandomGenerator>

struct SessionParticipant
{
    int     userId = 0;
    QString username;
    QString role;          // "Host" or "Member"
    int     currentPage = 1;
    bool    online = true;
};

struct SessionChatMessage
{
    int     senderId = 0;
    QString senderName;
    QString text;
    QString timestamp;
    int     colorIndex = 0;
};

struct ReadingSession
{
    int     sessionId = 0;
    QString sessionCode;
    int     bookId = 0;
    int     hostUserId = 0;
    QMap<int, SessionParticipant> participants;
    QVector<SessionChatMessage>   chatHistory;
};

class ReadingSessionService : public QObject
{
    Q_OBJECT
public:
    explicit ReadingSessionService(QObject* parent = nullptr);

    ReadingSession createSession(int bookId, int hostUserId, const QString& hostUsername);

    // outSession is populated on success; isNewParticipant tells the caller
    // whether this was a fresh join (so it knows whether to announce it to
    // other participants) or a reconnect of someone already in the session.
    bool joinSession(int sessionId, int userId, const QString& username,
                     ReadingSession& outSession, bool& isNewParticipant);

    bool leaveSession(int sessionId, int userId, bool& sessionEnded, int& newHostUserId);
    bool updatePage(int sessionId, int userId, int page);
    bool appendChat(int sessionId, const SessionChatMessage& msg);
    bool getSession(int sessionId, ReadingSession& outSession) const;
    int  findSessionIdByCode(const QString& code) const;

    // Every other participant's userId in a session, excluding excludeUserId.
    // Used by commands to know who to push targeted updates to.
    QVector<int> otherParticipantIds(int sessionId, int excludeUserId) const;

private:
    QMap<int, ReadingSession> m_sessions;
    int m_nextSessionId = 1;
    mutable QMutex m_mutex;
    QString generateSessionCode() const;
};

#endif // READINGSESSIONSERVICE_H
