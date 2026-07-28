#include "groupreadingwindow.h"
#include "Users/ui_groupreadingwindow.h"

#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QPointer>
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QClipboard>
#include <QGuiApplication>
#include <QScrollBar>

GroupReadingWindow::GroupReadingWindow(NetworkManager* networkManager,
                                       const QVariantMap& bookData,
                                       int sessionId,
                                       QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GroupReadingWindow)
    , m_networkManager(networkManager)
    , m_bookData(bookData)
    , m_sessionId(sessionId)
{
    ui->setupUi(this);

    m_bookId     = bookData.value("bookId").toInt();
    m_totalPages = bookData.value("totalPages").toInt();

    QString title  = bookData.value("title").toString();
    QString author = bookData.value("author").toString();
    ui->bookTitleLabel->setText(title + "  -  " + author);

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

    m_syncTimer = new QTimer(this);
    m_syncTimer->setInterval(SYNC_INTERVAL_MS);
    connect(m_syncTimer, &QTimer::timeout,
            this, &GroupReadingWindow::onSyncTimerTimeout);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &GroupReadingWindow::handleResponse);

    setupUiConnections();

    m_currentPage = 1;
    updatePageDisplay();
    loadBookCover();
    showSessionInfo();
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
    if (m_sessionId > 0) {
        // ---- join existing session ----
        QVariantMap data;
        data["sessionId"] = m_sessionId;
        data["userId"]    = SessionManager::instance()->getUserId();
        data["username"]  = SessionManager::instance()->getUsername();
        Request request(CommandType::JoinReadingSession, data);
        m_networkManager->sendRequest(request);
    }
}

void GroupReadingWindow::leaveSession()
{
    if (m_sessionId <= 0) return;

    QVariantMap data;
    data["sessionId"] = m_sessionId;
    data["userId"]    = SessionManager::instance()->getUserId();
    Request request(CommandType::LeaveReadingSession, data);
    m_networkManager->sendRequest(request);

    m_syncTimer->stop();
    m_sessionId = -1;
}

void GroupReadingWindow::on_createSessionButton_clicked()
{
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

    // copy invite code to clipboard
    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_sessionCode);
    QMessageBox::information(this, "Invite Code",
                             "Session invite code copied to clipboard:\n" + m_sessionCode);
}

void GroupReadingWindow::on_leaveSessionButton_clicked()
{
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Leave Session",
                              "Are you sure you want to leave this reading session?");
    if (reply == QMessageBox::Yes) {
        leaveSession();
        emit backRequested();
    }
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

            // add self to participants
            Participant self;
            self.userId     = SessionManager::instance()->getUserId();
            self.username   = SessionManager::instance()->getUsername();
            self.role       = "Host";
            self.currentPage = 1;
            self.totalPages  = m_totalPages;
            self.online     = true;
            self.color      = pickColor(0);
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

            // load participants from server
            QVariantList participants = response.getData()["participants"].toList();
            m_participants.clear();
            int colorIdx = 0;
            for (const QVariant& pv : participants) {
                QVariantMap pm  = pv.toMap();
                Participant p;
                p.userId      = pm["userId"].toInt();
                p.username    = pm["username"].toString();
                p.role        = pm["role"].toString();
                p.currentPage = pm["currentPage"].toInt();
                p.totalPages  = pm["totalPages"].toInt();
                p.online      = pm["online"].toBool();
                p.color       = pickColor(colorIdx++);
                m_participants[p.userId] = p;
            }

            showSessionInfo();
            updatePageDisplay();
            updateParticipantsList();
            addChatMessage({0, "System",
                            "You joined the reading session.",
                            QDateTime::currentDateTime().toString("HH:mm"),
                            QColor("gray")});

            m_syncTimer->start();
        } else {
            QMessageBox::warning(this, "Session",
                                 "Failed to join session: " + response.getMessage());
        }
        break;
    }

    case CommandType::ReadingSessionParticipantUpdate:
    {
        QVariantMap data = response.getData();
        int userId = data["userId"].toInt();
        bool joined = data["joined"].toBool();

        if (joined) {
            Participant p;
            p.userId      = userId;
            p.username    = data["username"].toString();
            p.role        = data["role"].toString();
            p.currentPage = data["currentPage"].toInt();
            p.totalPages  = m_totalPages;
            p.online      = true;
            assignUserColor(p);
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
        QVariantMap data = response.getData();
        int senderId  = data["senderId"].toInt();
        int page      = data["page"].toInt();

        if (m_participants.contains(senderId)) {
            m_participants[senderId].currentPage = page;
            updateParticipantsList();
        }

        if (!m_isHost && senderId == m_participants.value(m_sessionId).userId) {
            m_currentPage = page;
            updatePageDisplay();
        }

        emit remotePageChanged(senderId, page);
        break;
    }

    case CommandType::ReadingSessionFullSync:
    {
        QVariantList participants = response.getData()["participants"].toList();
        m_participants.clear();
        int colorIdx = 0;
        for (const QVariant& pv : participants) {
            QVariantMap pm  = pv.toMap();
            Participant p;
            p.userId      = pm["userId"].toInt();
            p.username    = pm["username"].toString();
            p.role        = pm["role"].toString();
            p.currentPage = pm["currentPage"].toInt();
            p.totalPages  = pm["totalPages"].toInt();
            p.online      = pm["online"].toBool();
            p.color       = pickColor(colorIdx++);
            m_participants[p.userId] = p;
        }
        updateParticipantsList();

        QVariantList chat = response.getData()["chatHistory"].toList();
        m_chatHistory.clear();
        ui->chatDisplayTextEdit->clear();
        for (const QVariant& cv : chat) {
            QVariantMap cm  = cv.toMap();
            ChatMessage msg;
            msg.senderId   = cm["senderId"].toInt();
            msg.senderName = cm["senderName"].toString();
            msg.text       = cm["text"].toString();
            msg.timestamp  = cm["timestamp"].toString();
            msg.color      = pickColor(cm["colorIndex"].toInt());
            addChatMessage(msg);
        }
        break;
    }

    case CommandType::ReadingSessionChat:
    {
        QVariantMap data = response.getData();
        ChatMessage msg;
        msg.senderId   = data["senderId"].toInt();
        msg.senderName = data["senderName"].toString();
        msg.text       = data["text"].toString();
        msg.timestamp  = QDateTime::currentDateTime().toString("HH:mm");
        msg.color      = pickColor(data["colorIndex"].toInt());

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
    QString text = ui->chatInputLineEdit->text().trimmed();
    if (text.isEmpty()) return;

    int myId = SessionManager::instance()->getUserId();
    QString myName = SessionManager::instance()->getUsername();
    int colorIndex = 0;
    if (m_participants.contains(myId)) {
        colorIndex = m_colorPool.indexOf(m_participants[myId].color);
    }

    // add locally
    ChatMessage msg;
    msg.senderId   = myId;
    msg.senderName = myName;
    msg.text       = text;
    msg.timestamp  = QDateTime::currentDateTime().toString("HH:mm");
    msg.color      = pickColor(colorIndex);
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

    // enable / disable navigation buttons
    ui->firstPageButton->setEnabled(m_currentPage > 1);
    ui->prevPageButton->setEnabled(m_currentPage > 1);
    ui->nextPageButton->setEnabled(m_currentPage < m_totalPages);
    ui->lastPageButton->setEnabled(m_currentPage < m_totalPages);

    // update status
    ui->statusLabel->setText(
        m_isHost
            ? "Host  -  Session active"
            : "Member  -  Following host");
}

void GroupReadingWindow::updateParticipantsList()
{
    ui->participantsListWidget->clear();

    // sort: online first, host first
    QList<Participant> sorted = m_participants.values();
    std::sort(sorted.begin(), sorted.end(),
              [](const Participant& a, const Participant& b) {
                  if (a.online != b.online) return a.online > b.online;
                  if (a.role == "Host") return true;
                  return a.username < b.username;
              });

    for (const Participant& p : sorted) {
        QString statusIcon = p.online ? "\u2705" : "\u26ab";
        QString roleIcon   = (p.role == "Host") ? "\uD83D\uDC51" : "\uD83D\uDC64";
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

    // update count label
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

    // auto-scroll to bottom
    QScrollBar* sb = ui->chatDisplayTextEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void GroupReadingWindow::showSessionInfo()
{
    if (m_sessionId > 0) {
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
}


void GroupReadingWindow::assignUserColor(Participant& p)
{
    // find first unused colour
    QSet<QString> usedColors;
    for (const Participant& existing : m_participants) {
        usedColors.insert(existing.color.name());
    }
    for (const QColor& c : m_colorPool) {
        if (!usedColors.contains(c.name())) {
            p.color = c;
            return;
        }
    }
    // fallback
    p.color = pickColor(m_participants.size() % m_colorPool.size());
}

QColor GroupReadingWindow::pickColor(int index)
{
    if (index < 0 || index >= m_colorPool.size())
        index = index % m_colorPool.size();
    return m_colorPool.at(index);
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
