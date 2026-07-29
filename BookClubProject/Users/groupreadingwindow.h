#ifndef GROUPREADINGWINDOW_H
#define GROUPREADINGWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QVariantMap>
#include <QPdfDocument>
#include <QtPdfWidgets/QPdfView>
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Shared/Book.h"
#include "../appWindow/SessionManager.h"
#include <QListWidgetItem>
#include <QLabel>

class QLineEdit;
class QPushButton;

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
                                QWidget *parent = nullptr);
    ~GroupReadingWindow();

    void setBookData(const QVariantMap& bookData, int sessionId = -1);
    // Join a session we already have a numeric sessionId for (e.g. constructed
    // with one, or resumed from a previous screen).
    void joinSession();

    // Join a session by its human-readable invite code (used by the
    // join-by-code UI, since that is the only thing non-hosts are given).
    void joinSessionByCode(const QString& code);

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
    void on_joinSessionButton_clicked();

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

    // Stable per-user color assignment. Colors are looked up by userId, never
    // by list/array position, so they can't drift when participant ordering
    // changes or two clients build their local list in a different order.
    QMap<int, QColor> m_userColorMap;

    QList<ChatMessage> m_chatHistory;

    QMap<int, QPixmap> m_coverCache;
    QMultiMap<int, QPointer<QLabel>> m_pendingCoverLabels;

    QTimer* m_syncTimer = nullptr;
    static constexpr int SYNC_INTERVAL_MS = 3000;

    // Join-by-code widgets. Built programmatically in setupUiConnections()
    // and inserted into sessionActionsFrame's layout, since the .ui file
    // has no control for a non-host to enter an invite code.
    QLineEdit*   m_joinCodeLineEdit  = nullptr;
    QPushButton* m_joinSessionButton = nullptr;

    // Actual PDF rendering for the book this session is reading, mirroring
    // PdfReaderWindow's setup. m_currentPage/m_totalPages here stay 1-based
    // (matching the rest of this class's existing convention), so they are
    // translated to/from QPdfView's 0-based page indices at the boundary.
    QPdfDocument* m_pdfDocument = nullptr;
    QPdfView*     m_pdfView     = nullptr;

    // Guards against feedback loops: set while we are programmatically
    // moving m_pdfView to match m_currentPage (from a toolbar click, a
    // remote host page-sync, or landing on a page after join/document-load).
    // While true, the pageNavigator's currentPageChanged handler treats the
    // resulting signal as "not a new local navigation" and does not
    // broadcast/re-emit it.
    bool m_applyingRemotePage = false;

    void setupUiConnections();
    void setupPdfView();
    bool loadBookPdf();
    void loadBookCover();
    void loadCoverInto(QLabel* label, int bookId, QSize targetSize);
    void updatePageDisplay();
    void updateParticipantsList();
    void addChatMessage(const ChatMessage& msg);
    void requestSync();
    void broadcastPageChange(int page);
    void showSessionInfo();
    void resetSessionState();

    // Identity-based color lookup (fixes color reassignment-by-position bug).
    QColor colorForUser(int userId);
};

#endif // GROUPREADINGWINDOW_H