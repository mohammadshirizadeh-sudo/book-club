#ifndef GROUPREADINGWINDOW_H
#define GROUPREADINGWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QVariantMap>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Book.h"
#include "../appWindow/SessionManager.h"

namespace Ui {
class GroupReadingWindow;
}

struct Participant
{
    int     userId       = 0;
    QString username;
    QString role;
    int     currentPage     = 0;
    int     totalPages      = 0;
    bool    online         = false;
    QColor  color;
};

struct ChatMessage
{
    int     senderId   = 0;
    QString senderName;
    QString text;
    QString timestamp;
    QColor  color;
};

class GroupReadingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GroupReadingWindow(NetworkManager* networkManager,
                                const QVariantMap& bookData,
                                int sessionId  = -1,
                                QWidget *parent = nullptr);
    ~GroupReadingWindow();

    void joinSession();
    void leaveSession();

    int  sessionId()   const { return m_sessionId; }
    int  currentPage() const { return m_currentPage; }
    bool isHost()      const { return m_isHost; }

signals:
    void backRequested();

    void localPageChanged(int page);

    void remotePageChanged(int userId, int page);

    void participantsChanged();

    void chatMessageReceived(const ChatMessage& msg);

private slots:
    void handleResponse(const Response& response);

    void on_backButton_clicked();
    void on_firstPageButton_clicked();
    void on_prevPageButton_clicked();
    void on_nextPageButton_clicked();
    void on_lastPageButton_clicked();
    void on_pageEdit_returnPressed();
    void on_pageJumpButton_clicked();

    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomCombo_currentIndexChanged(int index);
    void on_fullscreenButton_clicked();

    void on_createSessionButton_clicked();
    void on_inviteButton_clicked();
    void on_leaveSessionButton_clicked();

    void on_sendChatButton_clicked();
    void on_chatInput_returnPressed();

    void on_participantsListWidget_itemClicked(QListWidgetItem* item);

    void onSyncTimerTimeout();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::GroupReadingWindow* ui;
    NetworkManager*         m_networkManager;

    QVariantMap m_bookData;
    int         m_bookId     = 0;
    int         m_totalPages = 0;

    int  m_sessionId    = -1;
    bool m_isHost       = false;
    QString m_sessionCode;

    int  m_currentPage = 1;
    int  m_zoomLevel   = 100;

    QMap<int, Participant> m_participants;
    QList<QColor>            m_colorPool;

    QList<ChatMessage> m_chatHistory;

    QMap<int, QPixmap>             m_coverCache;
    QMultiMap<int, QPointer<QLabel>> m_pendingCoverLabels;

    QTimer* m_syncTimer = nullptr;
    static constexpr int SYNC_INTERVAL_MS = 3000;

    void setupUiConnections();
    void loadBookCover();
    void loadCoverInto(QLabel* label, int bookId, QSize targetSize);
    void updatePageDisplay();
    void updateParticipantsList();
    void addChatMessage(const ChatMessage& msg);
    void requestSync();
    void broadcastPageChange(int page);
    void showSessionInfo();
    void assignUserColor(Participant& p);
    QColor pickColor(int index);
};

#endif // GROUPREADINGWINDOW_H
