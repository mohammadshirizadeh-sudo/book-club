#include "groupreadingwindow.h"

#include "Users/ui_groupreadingwindow.h"

#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QPointer>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QClipboard>
#include <QGuiApplication>
#include <QScrollBar>
#include <QPdfDocument>
#include <QPdfPageNavigator>
#include <QtPdfWidgets/QPdfView>
#include <QFile>




GroupReadingWindow::GroupReadingWindow(NetworkManager* networkManager,
                                       QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GroupReadingWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    m_colorPool = {
        QColor("#e74c3c"), QColor("#3498db"), QColor("#2ecc71"),
        QColor("#f39c12"), QColor("#9b59b6"), QColor("#1abc9c"),
        QColor("#e67e22"), QColor("#e84393"), QColor("#00cec9"),
        QColor("#6c5ce7")
    };

    if (ui->zoomCombo->count() == 0) {
        ui->zoomCombo->addItems({"50%","75%","100%","125%","150%","200%","300%"});
        ui->zoomCombo->setCurrentIndex(2); // 100%
    }

    setupPdfView();

    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(SYNC_INTERVAL_MS);
    connect(m_syncTimer, &QTimer::timeout,
            this, &GroupReadingWindow::onSyncTimerTimeout);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &GroupReadingWindow::handleResponse);

    setupUiConnections();
}



void GroupReadingWindow::setBookData(const QVariantMap& bookData, int sessionId)
{
    m_bookData  = bookData;
    m_sessionId = sessionId;

    m_bookId     = m_bookData.value("bookId").toInt();
    m_totalPages = m_bookData.value("totalPages").toInt();

    QString title  = m_bookData.value("title").toString();
    QString author = m_bookData.value("author").toString();
    ui->bookTitleLabel->setText(title + "  -  " + author);

    m_currentPage = 1;
    loadBookCover();
    loadBookPdf();
    updatePageDisplay();
    showSessionInfo();

    if (m_sessionId > 0) {
        joinSession();
    }
}

GroupReadingWindow::~GroupReadingWindow()
{
    leaveSession();
    if (m_syncTimer) {
        m_syncTimer->stop();
    }
    delete ui;
}


void GroupReadingWindow::joinSession()
{
    if (m_sessionId <= 0) return;

    QVariantMap data;
    data["sessionId"] = m_sessionId;
    data["userId"]    = SessionManager::instance()->getUserId();
    data["username"]  = SessionManager::instance()->getUsername();
    Request request(CommandType::JoinReadingSession, data);
    m_networkManager->sendRequest(request);
}

void GroupReadingWindow::joinSessionByCode(const QString& code)
{
    if (code.trimmed().isEmpty()) return;

    QVariantMap data;
    data["sessionCode"] = code.trimmed().toUpper();
    data["userId"]      = SessionManager::instance()->getUserId();
    data["username"]    = SessionManager::instance()->getUsername();
    Request request(CommandType::JoinReadingSession, data);
    m_networkManager->sendRequest(request);
}

void GroupReadingWindow::leaveSession()
{
    if (m_sessionId <= 0) return;

    QVariantMap data;
    data["sessionId"] = m_sessionId;
    data["userId"]    = SessionManager::instance()->getUserId();
    Request request(CommandType::LeaveReadingSession, data);
    m_networkManager->sendRequest(request);

    // Fix #5: leaveSession() left every piece of session state stale
    // (session code, host flag, participants, chat, UI labels/lists).
    m_syncTimer->stop();
    resetSessionState();
    showSessionInfo();
    updateParticipantsList();
}

void GroupReadingWindow::resetSessionState()
{
    m_sessionId = -1;
    m_isHost = false;
    m_sessionCode.clear();
    m_participants.clear();
    m_chatHistory.clear();
    m_userColorMap.clear();
    if (ui->chatDisplayTextEdit) ui->chatDisplayTextEdit->clear();
    if (ui->participantsListWidget) ui->participantsListWidget->clear();
}

void GroupReadingWindow::on_createSessionButton_clicked()
{
    // Fix: defend against being opened without book context. The upstream
    // wiring (UserWindow::groubReadingWindow signal in main.cpp) currently
    // only flips the stack index - it never calls setBookData(), so m_bookId
    // stays 0 and every Create Session click sends bookId:0, which the
    // server rejects with "Missing required fields". The real fix belongs
    // upstream (pass the book through that signal), but in the meantime
    // fail fast here with an actionable message instead of firing a doomed
    // request.
    if (m_bookId <= 0) {
        QMessageBox::warning(this, "No Book Selected",
                             "Open this window from a specific book before creating a session.");
        return;
    }

    // Fix #7: creating a session while already in one clobbered the old
    // session's client-side state without ever leaving it server-side,
    // leaking a ghost session that still lists us as a participant.
    if (m_sessionId > 0) {
        QMessageBox::warning(this, "Session Active",
                             "Leave your current session before creating a new one.");
        return;
    }

    QVariantMap data;
    data["bookId"]   = m_bookId;
    data["userId"]   = SessionManager::instance()->getUserId();
    data["username"] = SessionManager::instance()->getUsername();
    data["role"]     = "Host";

    Request request(CommandType::CreateReadingSession, data);
    m_networkManager->sendRequest(request);
}

void GroupReadingWindow::on_inviteButton_clicked()
{
    if (m_sessionCode.isEmpty()) {
        QMessageBox::information(this, "Invite", "No active session to invite users to.");
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_sessionCode);
    QMessageBox::information(this, "Invite Code",
                             "Session invite code copied to clipboard:\n" + m_sessionCode);
}

void GroupReadingWindow::on_leaveSessionButton_clicked()
{
    // Fix #6: don't prompt for confirmation when there's nothing to leave.
    if (m_sessionId <= 0) {
        QMessageBox::information(this, "No Session", "You are not in an active session.");
        return;
    }

    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Leave Session",
                              "Are you sure you want to leave this reading session?");
    if (reply == QMessageBox::Yes) {
        leaveSession();
        emit backRequested();
    }
}

void GroupReadingWindow::on_joinSessionButton_clicked()
{
    if (!m_joinCodeLineEdit) return;

    QString code = m_joinCodeLineEdit->text().trimmed().toUpper();
    if (code.isEmpty()) {
        QMessageBox::information(this, "Join Session", "Enter a session code first.");
        return;
    }
    if (m_sessionId > 0) {
        QMessageBox::warning(this, "Session Active",
                             "Leave your current session before joining another.");
        return;
    }

    joinSessionByCode(code);
}


void GroupReadingWindow::handleResponse(const Response& response)
{
    switch (response.getCommandType()) {

    case CommandType::CreateReadingSession:
    {
        if (response.isSuccess()) {
            m_sessionId   = response.getData()["sessionId"].toInt();
            m_sessionCode = response.getData()["sessionCode"].toString();
            m_isHost      = true;

            Participant self;
            self.userId     = SessionManager::instance()->getUserId();
            self.username   = SessionManager::instance()->getUsername();
            self.role       = "Host";
            self.currentPage = 1;
            self.totalPages  = m_totalPages;
            self.online     = true;
            self.color      = colorForUser(self.userId);
            m_participants[self.userId] = self;

            showSessionInfo();
            updateParticipantsList();
            addChatMessage({0, "System",
                            "Reading session created. Invite code: " + m_sessionCode,
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});

            m_syncTimer->start();
        } else {
            QMessageBox::warning(this, "Session",
                                 "Failed to create session: " + response.getMessage());
        }
        break;
    }

    case CommandType::JoinReadingSession:
    {
        if (response.isSuccess()) {
            m_sessionId    = response.getData()["sessionId"].toInt();
            m_sessionCode  = response.getData()["sessionCode"].toString();
            m_isHost       = false;
            m_currentPage  = response.getData()["currentPage"].toInt();
            if (m_currentPage <= 0) m_currentPage = 1;

            // If we joined purely via an invite code, this window may never
            // have been given this book's data (title/cover/pdfPath) - the
            // only thing a non-host is handed is the code. Fetch it now.
            int sessionBookId = response.getData().value("bookId").toInt();
            if (sessionBookId > 0 && sessionBookId != m_bookId) {
                m_bookId = sessionBookId;
                QVariantMap params;
                params["bookId"] = m_bookId;
                params["userId"] = SessionManager::instance()->getUserId();
                Request bookRequest(CommandType::GetBookById, params);
                m_networkManager->sendRequest(bookRequest);
            }

            QVariantList participants = response.getData()["participants"].toList();
            m_participants.clear();
            for (const QVariant& pv : participants) {
                QVariantMap pm  = pv.toMap();
                Participant p;
                p.userId      = pm["userId"].toInt();
                p.username    = pm["username"].toString();
                p.role        = pm["role"].toString();
                p.currentPage = pm["currentPage"].toInt();
                // Server doesn't track totalPages per-book; we already know
                // it locally from the bookData this window was opened with.
                p.totalPages  = m_totalPages;
                p.online      = pm["online"].toBool();
                // Fix #4: identity-based color, not list-position based.
                p.color       = colorForUser(p.userId);
                m_participants[p.userId] = p;
            }

            // Chat history, if the server included it, is replayed exactly
            // once here (see fix #3 - it must NOT be replayed on periodic sync).
            if (response.getData().contains("chatHistory")) {
                QVariantList chat = response.getData()["chatHistory"].toList();
                m_chatHistory.clear();
                ui->chatDisplayTextEdit->clear();
                for (const QVariant& cv : chat) {
                    QVariantMap cm = cv.toMap();
                    ChatMessage msg;
                    msg.senderId   = cm["senderId"].toInt();
                    msg.senderName = cm["senderName"].toString();
                    msg.text       = cm["text"].toString();
                    msg.timestamp  = cm["timestamp"].toString();

                    // Fix: prefer the server-stored colorIndex (the color
                    // the sender originally chose for themselves) over a
                    // local recompute. Falls back to colorForUser() if the
                    // server omitted the field.
                    int colorIdx = cm.value("colorIndex", -1).toInt();
                    if (colorIdx >= 0 && colorIdx < m_colorPool.size()) {
                        msg.color = m_colorPool.at(colorIdx);
                    } else {
                        msg.color = colorForUser(msg.senderId);
                    }
                    addChatMessage(msg);
                }
            }

            showSessionInfo();
            updatePageDisplay();
            updateParticipantsList();
            addChatMessage({0, "System",
                            "You joined the reading session.",
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});

            if (m_joinCodeLineEdit) m_joinCodeLineEdit->clear();

            m_syncTimer->start();
        } else {
            QMessageBox::warning(this, "Session",
                                 "Failed to join session: " + response.getMessage());
        }
        break;
    }

    case CommandType::GetBookById:
    {
        // Only relevant here as a follow-up to a code-join that lacked book
        // context (see JoinReadingSession above). Ignore anything else -
        // e.g. a stale response for a book we've since moved away from.
        if (!response.isSuccess()) break;
        QVariantMap data = response.getData();
        if (data.value("bookId").toInt() != m_bookId) break;

        m_bookData = data;
        QString title  = data.value("title").toString();
        QString author = data.value("author").toString();
        ui->bookTitleLabel->setText(title + "  -  " + author);

        loadBookCover();
        loadBookPdf();
        break;
    }

    case CommandType::ReadingSessionParticipantUpdate:
    {
        // Fix: server-side errors (e.g. malformed push, dropped session
        // state) would otherwise flow through as if a real update had
        // arrived. An error response has no userId/joined keys, so the
        // code below would treat it as "user 0 left" and silently wipe
        // a random participant from the local list.
        if (!response.isSuccess()) break;

        QVariantMap data = response.getData();

        // A host may have left, promoting someone else. This can arrive
        // alongside (or instead of) a "left" notification, so handle it
        // independently of the joined/left branch below.
        if (data.contains("newHostId")) {
            int newHostId = data["newHostId"].toInt();
            if (m_participants.contains(newHostId)) {
                m_participants[newHostId].role = "Host";
            }
            if (newHostId == SessionManager::instance()->getUserId()) {
                m_isHost = true;
                updatePageDisplay();
                addChatMessage({0, "System",
                                "You are now the session host.",
                                QDateTime::currentDateTime().toString("HH:mm"),
                                QColor("gray")});
            }
        }

        int userId = data["userId"].toInt();

        // Fix #8: if the server broadcasts joins to everyone including the
        // joiner themselves, don't double up with the message already shown
        // from the Join/CreateReadingSession response.
        if (userId == SessionManager::instance()->getUserId()) {
            updateParticipantsList();
            break;
        }

        bool joined = data["joined"].toBool();
        // Fix: server now also pushes a ParticipantUpdate when an existing
        // participant reconnects (isNewParticipant == false on the server
        // side). Render it as a "X reconnected" system message and flip
        // their online flag, but do NOT add them as a brand-new
        // participant (they're already in m_participants).
        bool reconnected = data.value("reconnected", false).toBool();

        if (reconnected) {
            if (m_participants.contains(userId)) {
                m_participants[userId].online = true;
                // Refresh username/role in case they changed while offline.
                QString name = data.value("username").toString();
                if (!name.isEmpty()) {
                    m_participants[userId].username = name;
                }
                QString role = data.value("role").toString();
                if (!role.isEmpty()) {
                    m_participants[userId].role = role;
                }
            } else {
                // Edge case: server says "reconnected" but we never knew
                // about this user (e.g. we joined after they first
                // connected and missed the original join). Treat it as a
                // fresh join so the local list stays consistent.
                Participant p;
                p.userId      = userId;
                p.username    = data.value("username").toString();
                p.role        = data.value("role").toString();
                p.currentPage = data.value("currentPage").toInt();
                p.totalPages  = m_totalPages;
                p.online      = true;
                p.color       = colorForUser(userId);
                m_participants[userId] = p;
            }

            QString name = m_participants.value(userId).username;
            if (name.isEmpty()) name = "A user";
            addChatMessage({0, "System",
                            name + " reconnected.",
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});
        } else if (joined) {
            Participant p;
            p.userId      = userId;
            p.username    = data["username"].toString();
            p.role        = data["role"].toString();
            p.currentPage = data["currentPage"].toInt();
            p.totalPages  = m_totalPages;
            p.online      = true;
            p.color       = colorForUser(userId);
            m_participants[userId] = p;

            addChatMessage({0, "System",
                            p.username + " joined the session.",
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});
        } else {
            QString name = m_participants.value(userId).username;
            if (name.isEmpty()) name = "A user";
            m_participants.remove(userId);

            addChatMessage({0, "System",
                            name + " left the session.",
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});
        }

        updateParticipantsList();
        emit participantsChanged();
        break;
    }

    case CommandType::ReadingSessionPageSync:
    {
        // Fix: guard against error responses - otherwise an error push has
        // no senderId/page keys, senderId would resolve to 0, and the
        // "follow the host" branch below would refuse to act (good) but
        // emitRemotePageChanged(0, 0) would still fire misleadingly.
        if (!response.isSuccess()) break;

        QVariantMap data = response.getData();
        int senderId  = data["senderId"].toInt();
        int page      = data["page"].toInt();

        if (m_participants.contains(senderId)) {
            m_participants[senderId].currentPage = page;
            updateParticipantsList();
        }

        // Fix #1: "follow the host" was comparing m_sessionId (not a userId)
        // against a participant looked up by that same wrong key. The actual
        // intent is: only auto-follow page changes broadcast by the host.
        if (!m_isHost && m_participants.contains(senderId) &&
            m_participants[senderId].role == "Host") {
            m_currentPage = page;
            updatePageDisplay();
        }

        emit remotePageChanged(senderId, page);
        break;
    }

    case CommandType::ReadingSessionFullSync:
    {
        // Fix #3: this fires every SYNC_INTERVAL_MS. It must only refresh
        // participant positions - never touch chat history/UI, or the chat
        // pane flickers and re-scrolls every 3 seconds.
        //
        // Fix: guard against error responses. Without this, an error
        // (e.g. "Session not found" if the server restarted) would land
        // here with no participants key, m_participants.clear() would run,
        // and then populate from an empty list - silently wiping everyone
        // from the participant list until the next successful tick.
        if (!response.isSuccess()) break;

        QVariantList participants = response.getData()["participants"].toList();
        m_participants.clear();
        for (const QVariant& pv : participants) {
            QVariantMap pm  = pv.toMap();
            Participant p;
            p.userId      = pm["userId"].toInt();
            p.username    = pm["username"].toString();
            p.role        = pm["role"].toString();
            p.currentPage = pm["currentPage"].toInt();
            p.totalPages  = m_totalPages;
            p.online      = pm["online"].toBool();
            p.color       = colorForUser(p.userId);
            m_participants[p.userId] = p;
        }
        updateParticipantsList();
        // chatHistory intentionally NOT replayed here - see fix #3.
        break;
    }

    case CommandType::ReadingSessionChat:
    {
        // Fix: guard against error responses. An error response has no
        // senderId key, so without this guard senderId would resolve to 0,
        // the "this was my own message" check below would not match (our
        // own userId is never 0), and the message would render as coming
        // from a phantom user (id 0 / empty name).
        if (!response.isSuccess()) break;

        QVariantMap data = response.getData();
        int senderId = data["senderId"].toInt();

        // Fix #2: we already added our own message locally when we sent it;
        // the server's broadcast of it coming back would duplicate it.
        if (senderId == SessionManager::instance()->getUserId()) break;

        ChatMessage msg;
        msg.senderId   = senderId;
        msg.senderName = data["senderName"].toString();
        msg.text       = data["text"].toString();
        msg.timestamp  = QDateTime::currentDateTime().toString("HH:mm");

        // Fix: use the colorIndex the sender actually computed (and the
        // server round-tripped) instead of recomputing locally. Falls back
        // to colorForUser() if the server omitted it for any reason.
        int colorIdx = data.value("colorIndex", -1).toInt();
        if (colorIdx >= 0 && colorIdx < m_colorPool.size()) {
            msg.color = m_colorPool.at(colorIdx);
        } else {
            msg.color = colorForUser(senderId);
        }

        addChatMessage(msg);
        emit chatMessageReceived(msg);
        break;
    }

    case CommandType::GetBookCover:
    {
        if (!response.isSuccess()) return;
        int bookId = response.getData()["bookId"].toInt();
        QByteArray raw = QByteArray::fromBase64(
            response.getData()["coverData"].toByteArray());
        QPixmap pixmap;
        pixmap.loadFromData(raw);
        if (pixmap.isNull()) return;
        m_coverCache[bookId] = pixmap;
        for (QPointer<QLabel> label : m_pendingCoverLabels.values(bookId)) {
            if (label) {
                label->setPixmap(
                    pixmap.scaled(label->size(),
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation));
            }
        }
        m_pendingCoverLabels.remove(bookId);
        break;
    }

    default:
        break;
    }
}

void GroupReadingWindow::on_backButton_clicked()
{
    emit backRequested();
}

void GroupReadingWindow::on_firstPageButton_clicked()
{
    m_currentPage = 1;
    updatePageDisplay();
    broadcastPageChange(m_currentPage);
    emit localPageChanged(m_currentPage);
}

void GroupReadingWindow::on_prevPageButton_clicked()
{
    if (m_currentPage > 1) {
        m_currentPage--;
        updatePageDisplay();
        broadcastPageChange(m_currentPage);
        emit localPageChanged(m_currentPage);
    }
}

void GroupReadingWindow::on_nextPageButton_clicked()
{
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        updatePageDisplay();
        broadcastPageChange(m_currentPage);
        emit localPageChanged(m_currentPage);
    }
}

void GroupReadingWindow::on_lastPageButton_clicked()
{
    m_currentPage = m_totalPages;
    updatePageDisplay();
    broadcastPageChange(m_currentPage);
    emit localPageChanged(m_currentPage);
}

void GroupReadingWindow::on_pageEdit_returnPressed()
{
    bool ok = false;
    int page = ui->pageEdit->text().toInt(&ok);
    if (ok && page >= 1 && page <= m_totalPages) {
        m_currentPage = page;
        updatePageDisplay();
        broadcastPageChange(m_currentPage);
        emit localPageChanged(m_currentPage);
    } else {
        ui->pageEdit->setText(QString::number(m_currentPage));
    }
}

void GroupReadingWindow::on_pageJumpButton_clicked()
{
    on_pageEdit_returnPressed();
}


void GroupReadingWindow::on_zoomOutButton_clicked()
{
    int idx = ui->zoomCombo->currentIndex();
    if (idx > 0) {
        ui->zoomCombo->setCurrentIndex(idx - 1);
        on_zoomCombo_currentIndexChanged(idx - 1);
    }
}

void GroupReadingWindow::on_zoomInButton_clicked()
{
    int idx = ui->zoomCombo->currentIndex();
    if (idx < ui->zoomCombo->count() - 1) {
        ui->zoomCombo->setCurrentIndex(idx + 1);
        on_zoomCombo_currentIndexChanged(idx + 1);
    }
}

void GroupReadingWindow::on_zoomCombo_currentIndexChanged(int index)
{
    if (index < 0) return;
    QString text = ui->zoomCombo->itemText(index);
    m_zoomLevel = text.replace("%","").toInt();
    ui->zoomStatusLabel->setText("Zoom: " + text);
    if (m_pdfView) {
        m_pdfView->setZoomFactor(m_zoomLevel / 100.0);
    }
}

void GroupReadingWindow::on_fullscreenButton_clicked()
{
    if (isFullScreen()) {
        showNormal();
        ui->fullscreenButton->setText("\u29c9");
    } else {
        showFullScreen();
        ui->fullscreenButton->setText("\u29d1");
    }
}

void GroupReadingWindow::on_sendChatButton_clicked()
{
    // Fix: don't send chat when not in a session. Previously the message
    // would be appended locally (so the user thought it was sent) and then
    // the server would reject it - and because the ReadingSessionChat
    // handler didn't check isSuccess(), the rejection was silently
    // swallowed. Now we fail fast up front instead.
    if (m_sessionId <= 0) {
        QMessageBox::information(this, "Chat",
                                 "Join or create a session before sending chat messages.");
        return;
    }

    QString text = ui->chatInputLineEdit->text().trimmed();
    if (text.isEmpty()) return;

    int myId = SessionManager::instance()->getUserId();
    QString myName = SessionManager::instance()->getUsername();
    QColor myColor = colorForUser(myId);
    int colorIndex = m_colorPool.indexOf(myColor);
    if (colorIndex < 0) colorIndex = 0;

    // add locally
    ChatMessage msg;
    msg.senderId   = myId;
    msg.senderName = myName;
    msg.text       = text;
    msg.timestamp  = QDateTime::currentDateTime().toString("HH:mm");
    msg.color      = myColor;
    addChatMessage(msg);

    // send to server
    QVariantMap data;
    data["sessionId"]  = m_sessionId;
    data["senderId"]   = myId;
    data["senderName"] = myName;
    data["text"]       = text;
    data["colorIndex"] = colorIndex;
    Request request(CommandType::ReadingSessionChat, data);
    m_networkManager->sendRequest(request);

    ui->chatInputLineEdit->clear();
    ui->chatInputLineEdit->setFocus();
}

void GroupReadingWindow::on_chatInput_returnPressed()
{
    on_sendChatButton_clicked();
}


void GroupReadingWindow::on_participantsListWidget_itemClicked(QListWidgetItem* item)
{
    int userId = item->data(Qt::UserRole).toInt();
    if (m_participants.contains(userId)) {
        const Participant& p = m_participants[userId];
        QString info = QString(
                           "Username: %1\n"
                           "Role: %2\n"
                           "Current Page: %3 / %4\n"
                           "Status: %5"
                           ).arg(p.username)
                           .arg(p.role)
                           .arg(p.currentPage)
                           .arg(p.totalPages)
                           .arg(p.online ? "Online" : "Offline");

        QMessageBox::information(this, "Participant Info", info);
    }
}


void GroupReadingWindow::onSyncTimerTimeout()
{
    requestSync();
}

void GroupReadingWindow::requestSync()
{
    if (m_sessionId <= 0) return;

    QVariantMap data;
    data["sessionId"] = m_sessionId;
    data["userId"]    = SessionManager::instance()->getUserId();
    data["currentPage"] = m_currentPage;
    Request request(CommandType::ReadingSessionFullSync, data);
    m_networkManager->sendRequest(request);
}

void GroupReadingWindow::broadcastPageChange(int page)
{
    if (m_sessionId <= 0) return;

    QVariantMap data;
    data["sessionId"] = m_sessionId;
    data["senderId"]  = SessionManager::instance()->getUserId();
    data["page"]      = page;
    Request request(CommandType::ReadingSessionPageSync, data);
    m_networkManager->sendRequest(request);
}


void GroupReadingWindow::setupUiConnections()
{
    // Missing feature: there was no way for anyone but the host (who has
    // sessionId already) to join a session - only a code, never a numeric
    // id, is available to invitees. Build a small join-by-code control and
    // place it inside sessionActionsFrame.
    //
    // Fix (overlap bug): sessionActionsFrame in the .ui file uses Designer's
    // absolute-positioning mode (every child has hand-set geometry, no layout).
    // The previous implementation attached a brand-new QVBoxLayout to the
    // frame, which caused Qt to stretch that layout's contents across the
    // entire frame content rect - right on top of the existing absolutely-
    // positioned buttons (createSessionButton, inviteButton, etc.). Now we
    // place the new widgets with setGeometry() on a second manually-
    // positioned row inside the (now taller, 115px) frame, consistent with
    // the rest of the file. No QLayout is ever installed on sessionActionsFrame.
    if (ui->sessionActionsFrame) {
        m_joinCodeLineEdit = new QLineEdit(ui->sessionActionsFrame);
        m_joinCodeLineEdit->setObjectName("joinCodeLineEdit");
        m_joinCodeLineEdit->setPlaceholderText("Enter invite code");
        m_joinCodeLineEdit->setMaxLength(6);
        m_joinCodeLineEdit->setGeometry(10, 60, 300, 45);

        m_joinSessionButton = new QPushButton("Join Session", ui->sessionActionsFrame);
        m_joinSessionButton->setObjectName("joinSessionButton");
        m_joinSessionButton->setGeometry(320, 60, 300, 45);

        connect(m_joinSessionButton, &QPushButton::clicked,
                this, &GroupReadingWindow::on_joinSessionButton_clicked);
        connect(m_joinCodeLineEdit, &QLineEdit::returnPressed,
                this, &GroupReadingWindow::on_joinSessionButton_clicked);
    }

    // Fix (dead "Enter to send"): the line edit's object name is
    // chatInputLineEdit, so Qt's auto-connect expected
    // on_chatInputLineEdit_returnPressed - the on_chatInput_returnPressed
    // slot was never wired. Wire it explicitly here so pressing Enter in
    // the chat box sends the message (matching the send button's behaviour).
    if (ui->chatInputLineEdit) {
        connect(ui->chatInputLineEdit, &QLineEdit::returnPressed,
                this, &GroupReadingWindow::on_chatInput_returnPressed);
    }
}

void GroupReadingWindow::setupPdfView()
{
    m_pdfDocument = new QPdfDocument(this);

    m_pdfView = new QPdfView(ui->pdfViewContainer);
    m_pdfView->setDocument(m_pdfDocument);
    m_pdfView->setPageMode(QPdfView::PageMode::MultiPage);
    m_pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    m_pdfView->setZoomFactor(1.0);
    m_pdfView->hide(); // placeholderLabel stays visible until a book/session is loaded

    QVBoxLayout* layout = new QVBoxLayout(ui->pdfViewContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pdfView);

    connect(m_pdfDocument, &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status status) {
        if (status == QPdfDocument::Status::Ready) {
            ui->placeholderLabel->hide();
            m_pdfView->show();
            int docPages = m_pdfDocument->pageCount();
            if (docPages > 0) m_totalPages = docPages;
            // Lands the view on whatever page we already know about (e.g.
            // the host's current page, learned from the join response,
            // before the PDF itself had finished loading).
            updatePageDisplay();
        } else if (status == QPdfDocument::Status::Error) {
            m_pdfView->hide();
            ui->placeholderLabel->setText("Failed to load this book's PDF.");
            ui->placeholderLabel->show();
        }
    });

    // Fires both when we programmatically jump() and when the user scrolls
    // the view by hand. m_applyingRemotePage tells us which case this is.
    connect(m_pdfView->pageNavigator(), &QPdfPageNavigator::currentPageChanged,
            this, [this](int page) {
                int newPage = page + 1; // this class's pages are 1-based
                if (newPage == m_currentPage) return;
                m_currentPage = newPage;
                updatePageDisplay();
                if (!m_applyingRemotePage) {
                    // Real local navigation - the user scrolled the PDF
                    // itself rather than using a toolbar button. Treat it
                    // exactly like a toolbar page change.
                    broadcastPageChange(m_currentPage);
                    emit localPageChanged(m_currentPage);
                }
            });
}

bool GroupReadingWindow::loadBookPdf()
{
    QString pdfPath = m_bookData.value("pdfPath").toString();
    if (pdfPath.isEmpty()) {
        m_pdfView->hide();
        ui->placeholderLabel->setText("This book has no readable file attached.");
        ui->placeholderLabel->show();
        return false;
    }
    if (!QFile::exists(pdfPath)) {
        m_pdfView->hide();
        ui->placeholderLabel->setText("PDF file not found on server.");
        ui->placeholderLabel->show();
        return false;
    }

    ui->placeholderLabel->setText("Loading PDF...");
    ui->placeholderLabel->show();

    QPdfDocument::Error err = m_pdfDocument->load(pdfPath);
    return err == QPdfDocument::Error::None;
}

void GroupReadingWindow::loadBookCover()
{
    QString coverPath = m_bookData.value("coverPath").toString();
    if (!coverPath.isEmpty()) {
        loadCoverInto(ui->bookCoverLabel, m_bookId, QSize(60, 85));
    }
}

void GroupReadingWindow::loadCoverInto(QLabel* label, int bookId, QSize targetSize)
{
    if (m_coverCache.contains(bookId)) {
        label->setPixmap(
            m_coverCache[bookId].scaled(targetSize,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
        return;
    }

    label->setText("...");
    m_pendingCoverLabels.insert(bookId, label);
    m_networkManager->requestBookCover(bookId);
}

void GroupReadingWindow::updatePageDisplay()
{
    ui->pageEdit->setText(QString::number(m_currentPage));
    ui->pageInfoLabel->setText(
        QString("Page %1 / %2").arg(m_currentPage).arg(m_totalPages));

    ui->firstPageButton->setEnabled(m_currentPage > 1);
    ui->prevPageButton->setEnabled(m_currentPage > 1);
    ui->nextPageButton->setEnabled(m_currentPage < m_totalPages);
    // The .ui file now declares lastPageButton (previously the slot existed
    // but no widget did, so the auto-connect silently failed and the only
    // way to jump to the last page was the End key). Mirror the
    // firstPageButton enable/disable rule.
    if (ui->lastPageButton) {
        ui->lastPageButton->setEnabled(m_currentPage < m_totalPages);
    }

    ui->statusLabel->setText(
        m_isHost
            ? "Host  -  Session active"
            : "Member  -  Following host");

    // Keep the rendered PDF page in lock-step with m_currentPage, no matter
    // which path changed it (toolbar button, remote host sync, or landing
    // on a page once the document finishes loading). m_applyingRemotePage
    // stops the pageNavigator's currentPageChanged handler above from
    // treating this programmatic jump as a fresh local navigation.
    if (m_pdfView && m_pdfDocument &&
        m_pdfDocument->status() == QPdfDocument::Status::Ready) {
        int targetIndex = m_currentPage - 1;
        if (targetIndex >= 0 && m_pdfView->pageNavigator()->currentPage() != targetIndex) {
            m_applyingRemotePage = true;
            m_pdfView->pageNavigator()->jump(targetIndex, QPointF(), 0.0);
            m_applyingRemotePage = false;
        }
    }
}

void GroupReadingWindow::updateParticipantsList()
{
    ui->participantsListWidget->clear();

    QList<Participant> sorted = m_participants.values();
    std::sort(sorted.begin(), sorted.end(),
              [](const Participant& a, const Participant& b) {
                  if (a.online != b.online) return a.online > b.online;
                  if (a.role == "Host") return true;
                  return a.username < b.username;
              });

    for (const Participant& p : sorted) {
        QString statusIcon = p.online ? "\u2705" : "\u26ab";
        QString roleIcon = (p.role == "Host")
                               ? QString::fromUtf8("👑")
                               : QString::fromUtf8("👤");

        QString display = QString("%1 %2  %3  (Page %4/%5)")
                              .arg(statusIcon)
                              .arg(roleIcon)
                              .arg(p.username)
                              .arg(p.currentPage)
                              .arg(p.totalPages);

        QListWidgetItem* item = new QListWidgetItem(display);
        item->setData(Qt::UserRole, p.userId);
        item->setForeground(p.color);
        ui->participantsListWidget->addItem(item);
    }

    int onlineCount = 0;
    for (const Participant& p : m_participants) {
        if (p.online) onlineCount++;
    }
    ui->participantCountLabel->setText(
        QString("Participants: %1 online / %2 total")
            .arg(onlineCount)
            .arg(m_participants.size()));
}

void GroupReadingWindow::addChatMessage(const ChatMessage& msg)
{
    m_chatHistory.append(msg);

    QString line;
    if (msg.senderName == "System") {
        line = QString("<span style='color:gray;'>[%1] %2</span>")
        .arg(msg.timestamp, msg.text);
    } else {
        line = QString("<span style='color:%3;'>[%1] <b>%2:</b> %4</span>")
        .arg(msg.timestamp, msg.senderName, msg.color.name(), msg.text);
    }

    ui->chatDisplayTextEdit->append(line);

    QScrollBar* sb = ui->chatDisplayTextEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void GroupReadingWindow::showSessionInfo()
{
    const bool inSession = (m_sessionId > 0);

    if (inSession) {
        ui->sessionCodeLabel->setText("Session: " + m_sessionCode);
        ui->sessionStatusLabel->setText("Connected");
        ui->sessionStatusLabel->setStyleSheet(
            "color: #2ecc71; font-weight: bold;");
    } else {
        ui->sessionCodeLabel->setText("No Active Session");
        ui->sessionStatusLabel->setText("Disconnected");
        ui->sessionStatusLabel->setStyleSheet(
            "color: #e74c3c; font-weight: bold;");
    }

    // Fix: reflect session state in button enabledness up front rather
    // than letting the user click through to a warning dialog. "Create
    // Session" and "Join Session" are only meaningful when not already in
    // a session; "Invite" and "Leave" are only meaningful when in one.
    // The join-by-code widgets are built lazily in setupUiConnections(),
    // so they may not exist yet on the very first call.
    if (ui->createSessionButton) {
        ui->createSessionButton->setEnabled(!inSession);
    }
    if (ui->inviteButton) {
        ui->inviteButton->setEnabled(inSession);
    }
    if (ui->leaveSessionButton) {
        ui->leaveSessionButton->setEnabled(inSession);
    }
    if (m_joinSessionButton) {
        m_joinSessionButton->setEnabled(!inSession);
    }
    if (m_joinCodeLineEdit) {
        m_joinCodeLineEdit->setEnabled(!inSession);
    }
}

QColor GroupReadingWindow::colorForUser(int userId)
{
    // Fix: derive each user's color directly from their userId, not from
    // local insertion order into m_userColorMap. The previous
    // implementation (m_colorPool.at(m_userColorMap.size() % pool.size()))
    // assigned colors by insertion order, which meant two clients that
    // built their local participant list in different orders (e.g. one
    // received a full sync, another received only an incremental update
    // for a newly-joined user) could end up assigning different colors to
    // the same person - the comment above the old code claimed the
    // assignment was cross-client stable, but that guarantee did not
    // actually hold in the general case.
    //
    // Using userId % pool.size() makes the assignment a pure function of
    // the user's identity, so every client always picks the same color for
    // the same person, regardless of the order in which they learned about
    // participants. m_userColorMap is kept only as a cache.
    auto it = m_userColorMap.constFind(userId);
    if (it != m_userColorMap.constEnd()) {
        return it.value();
    }
    QColor c = m_colorPool.at(userId % m_colorPool.size());
    m_userColorMap[userId] = c;
    return c;
}


void GroupReadingWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        on_prevPageButton_clicked();
        break;
    case Qt::Key_Right:
        on_nextPageButton_clicked();
        break;
    case Qt::Key_Home:
        on_firstPageButton_clicked();
        break;
    case Qt::Key_End:
        on_lastPageButton_clicked();
        break;
    case Qt::Key_Escape:
        on_backButton_clicked();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}