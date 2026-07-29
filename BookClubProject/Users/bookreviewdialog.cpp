#include "bookreviewdialog.h"
#include "ui_bookreviewdialog.h"
#include "../appWindow/SessionManager.h"

#include <QMessageBox>
#include <QPixmap>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QDateTime>
#include <QTextCursor>
#include <algorithm>

BookReviewDialog::BookReviewDialog(NetworkManager* networkManager,
                                   const QVariantMap& bookData,
                                   QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BookReviewDialog)
    , m_networkManager(networkManager)
    , m_bookData(bookData)
{
    ui->setupUi(this);

    m_bookId = m_bookData.value("bookId").toInt();

    // stars act as a 1-5 "fill up to here" selector, not mutually exclusive radios
    ui->star1Button->setCheckable(true);
    ui->star2Button->setCheckable(true);
    ui->star3Button->setCheckable(true);
    ui->star4Button->setCheckable(true);
    ui->star5Button->setCheckable(true);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &BookReviewDialog::handleResponse);

    setupBookInfo();
    setStarRating(0);
    ui->previousRatingLabel->setText("");
    updateSubmitButtonState();
    on_reviewTextEdit_textChanged(); // initialize char counter to "0 / 1000"

    if (m_bookId > 0) {
        requestReviews();
    } else {
        qWarning() << "BookReviewDialog opened with an invalid bookId";
    }
}

BookReviewDialog::~BookReviewDialog()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &BookReviewDialog::handleResponse);
    delete ui;
}

void BookReviewDialog::setupBookInfo()
{
    ui->bookTitleLabel->setText(m_bookData.value("title").toString());
    ui->authorNameLabel->setText(m_bookData.value("author").toString());

    // seed with whatever caller already knew; requestReviews() will refresh
    // this with the authoritative count once GetReviewsForBook responds.
    double avg = m_bookData.value("averageRating").toDouble();
    ui->avgRatingDisplay->setText(QString("⭐ %1 (loading...)").arg(avg, 0, 'f', 1));

    QString coverPath = m_bookData.value("coverPath").toString();
    if (!coverPath.isEmpty() && m_bookId > 0) {
        ui->bookCoverLabel->setText("...");
        m_networkManager->requestBookCover(m_bookId);
    }
}

void BookReviewDialog::requestReviews()
{
    QVariantMap params;
    params["bookId"] = m_bookId;

    Request request(CommandType::GetReviewsForBook, params);
    m_networkManager->sendRequest(request);
}

void BookReviewDialog::handleResponse(const Response& response)
{
    switch (response.getCommandType()) {

    case CommandType::GetReviewsForBook:
    {
        if (!response.isSuccess()) {
            QMessageBox::warning(this, "Error", "Failed to load reviews: " + response.getMessage());
            return;
        }

        QVariantMap data = response.getData();
        double avg = data.value("averageRating").toDouble();
        int count = data.value("count").toInt();
        ui->avgRatingDisplay->setText(
            QString("⭐ %1 (%2 review%3)")
                .arg(avg, 0, 'f', 1)
                .arg(count)
                .arg(count == 1 ? "" : "s"));

        QVariantList reviews = data.value("reviews").toList();
        populateReviews(reviews);

        // Find whether the current user already reviewed this book, so
        // Submit becomes an edit instead of creating a duplicate. The
        // server enforces one review per (user, book) via a UNIQUE
        // constraint anyway, but pre-filling avoids a guaranteed-failing
        // AddReview round trip.
        int myId = SessionManager::instance()->getUserId();
        m_hasExistingReview = false;
        m_existingReviewId = -1;

        for (const QVariant& v : reviews) {
            QVariantMap r = v.toMap();
            if (r.value("userId").toInt() == myId) {
                m_hasExistingReview = true;
                m_existingReviewId  = r.value("reviewId").toInt();
                m_existingRating    = r.value("rating").toInt();

                ui->reviewTextEdit->setPlainText(r.value("text").toString());
                setStarRating(m_existingRating);
                ui->previousRatingLabel->setText(
                    QString("You previously rated this %1★").arg(m_existingRating));
                ui->submitReviewButton->setText("✏️ Update Review");
                break;
            }
        }

        if (!m_hasExistingReview) {
            ui->previousRatingLabel->setText("");
            ui->submitReviewButton->setText("✅ Submit");
        }

        updateSubmitButtonState();
        break;
    }

    case CommandType::AddReview:
    {
        ui->submitReviewButton->setEnabled(true);
        if (response.isSuccess()) {
            QMessageBox::information(this, "Review", "Your review was submitted. Thank you!");
            requestReviews(); // refresh list + flip into "edit" mode for next time
        } else {
            QMessageBox::warning(this, "Error", "Could not submit review: " + response.getMessage());
        }
        break;
    }

    case CommandType::EditReview:
    {
        ui->submitReviewButton->setEnabled(true);
        if (response.isSuccess()) {
            QMessageBox::information(this, "Review", "Your review was updated.");
            requestReviews();
        } else {
            QMessageBox::warning(this, "Error", "Could not update review: " + response.getMessage());
        }
        break;
    }

    case CommandType::GetBookCover:
    {
        if (!response.isSuccess()) return;
        if (response.getData().value("bookId").toInt() != m_bookId) return;

        QByteArray raw = QByteArray::fromBase64(response.getData()["coverData"].toByteArray());
        QPixmap pixmap;
        if (pixmap.loadFromData(raw)) {
            ui->bookCoverLabel->setPixmap(
                pixmap.scaled(ui->bookCoverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->bookCoverLabel->setText("");
        }
        break;
    }

    default:
        break;
    }
}

void BookReviewDialog::setStarRating(int rating)
{
    m_selectedRating = rating;

    QList<QPushButton*> stars = {
        ui->star1Button, ui->star2Button, ui->star3Button,
        ui->star4Button, ui->star5Button
    };

    for (int i = 0; i < stars.size(); ++i) {
        bool filled = (i < rating);
        stars[i]->setChecked(filled);
        stars[i]->setText(filled ? "★" : "☆");
    }

    updateSubmitButtonState();
}

void BookReviewDialog::updateSubmitButtonState()
{
    bool hasRating = (m_selectedRating >= 1 && m_selectedRating <= 5);
    ui->submitReviewButton->setEnabled(hasRating);
}

void BookReviewDialog::on_star1Button_clicked() { setStarRating(1); }
void BookReviewDialog::on_star2Button_clicked() { setStarRating(2); }
void BookReviewDialog::on_star3Button_clicked() { setStarRating(3); }
void BookReviewDialog::on_star4Button_clicked() { setStarRating(4); }
void BookReviewDialog::on_star5Button_clicked() { setStarRating(5); }

void BookReviewDialog::on_reviewTextEdit_textChanged()
{
    int len = ui->reviewTextEdit->toPlainText().length();

    if (len > MAX_REVIEW_LENGTH) {
        QString trimmed = ui->reviewTextEdit->toPlainText().left(MAX_REVIEW_LENGTH);
        ui->reviewTextEdit->blockSignals(true);
        ui->reviewTextEdit->setPlainText(trimmed);
        QTextCursor c = ui->reviewTextEdit->textCursor();
        c.movePosition(QTextCursor::End);
        ui->reviewTextEdit->setTextCursor(c);
        ui->reviewTextEdit->blockSignals(false);
        len = MAX_REVIEW_LENGTH;
    }

    ui->charCountLabel->setText(
        QString("<html><head/><body><p align=\"right\">%1 / %2 characters</p></body></html>")
            .arg(len).arg(MAX_REVIEW_LENGTH));
}

void BookReviewDialog::on_submitReviewButton_clicked()
{
    requestAddOrEditReview();
}

void BookReviewDialog::requestAddOrEditReview()
{
    if (m_selectedRating < 1 || m_selectedRating > 5) {
        QMessageBox::information(this, "Rating Required", "Please select a star rating before submitting.");
        return;
    }

    // NOTE: ReviewService::addReview() rejects empty text server-side
    // ("Review text cannot be empty"), even though this dialog's
    // placeholder says the review is "(Optional)". Enforce that here too
    // so the user gets an immediate, local message instead of a round
    // trip that's guaranteed to fail.
    QString text = ui->reviewTextEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, "Review Required",
                                 "Please write a few words about the book before submitting.");
        return;
    }

    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) {
        QMessageBox::warning(this, "Not Signed In", "You must be signed in to submit a review.");
        return;
    }

    QVariantMap params;
    params["userId"] = userId;
    params["bookId"] = m_bookId;
    params["text"]   = text;
    params["rating"] = m_selectedRating;

    ui->submitReviewButton->setEnabled(false);

    if (m_hasExistingReview && m_existingReviewId > 0) {
        params["reviewId"] = m_existingReviewId;
        Request request(CommandType::EditReview, params);
        m_networkManager->sendRequest(request);
    } else {
        Request request(CommandType::AddReview, params);
        m_networkManager->sendRequest(request);
    }
}

QString BookReviewDialog::formatTimeAgo(const QDateTime& dt) const
{
    if (!dt.isValid()) return "";

    qint64 secs = dt.secsTo(QDateTime::currentDateTime());
    if (secs < 60) return "Just now";
    if (secs < 3600) return QString::number(secs / 60) + " mins ago";
    if (secs < 86400) return QString::number(secs / 3600) + " hours ago";
    return QString::number(secs / 86400) + " days ago";
}

void BookReviewDialog::clearReviewsContainer()
{
    QLayout* layout = ui->reviewsContainerWidget->layout();
    if (!layout) return;

    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void BookReviewDialog::populateReviews(const QVariantList& reviews)
{
    clearReviewsContainer();

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->reviewsContainerWidget->layout());
    if (!layout) {
        layout = new QVBoxLayout(ui->reviewsContainerWidget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(10);
    }

    if (reviews.isEmpty()) {
        QLabel* empty = new QLabel("No reviews yet. Be the first to share your thoughts!");
        empty->setStyleSheet("color: gray; font-size: 16px; background: transparent; border: none;");
        empty->setAlignment(Qt::AlignCenter);
        layout->addWidget(empty);
        return;
    }

    QVariantList sorted = reviews;
    std::sort(sorted.begin(), sorted.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap().value("createdAt").toString() > b.toMap().value("createdAt").toString();
    });

    for (const QVariant& v : sorted) {
        QVariantMap r = v.toMap();
        int rating = r.value("rating").toInt();
        QString text = r.value("text").toString();
        QDateTime createdAt = QDateTime::fromString(r.value("createdAt").toString(), Qt::ISODate);

        QFrame* card = new QFrame();
        card->setStyleSheet(
            "QFrame { border: 1px solid rgba(0,0,0,80); border-radius: 8px; "
            "background: rgba(255,255,255,120); }");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);

        QString starsStr = QString("★").repeated(rating) + QString("☆").repeated(5 - rating);
        QLabel* header = new QLabel(QString("%1   %2").arg(starsStr, formatTimeAgo(createdAt)));
        header->setStyleSheet("color: rgb(255,193,7); font-size: 18px; background: transparent; border: none;");

        QLabel* body = new QLabel(text.isEmpty() ? "(No written review)" : text);
        body->setWordWrap(true);
        body->setStyleSheet("color: black; font-size: 14px; background: transparent; border: none;");

        cardLayout->addWidget(header);
        cardLayout->addWidget(body);
        layout->addWidget(card);
    }

    layout->addStretch();
}

void BookReviewDialog::on_closeButton_clicked()
{
    reject();
}