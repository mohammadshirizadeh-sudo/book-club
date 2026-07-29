
#include "favoritebookswindow.h"
#include "Users/ui_favoritebookswindow.h"
#include "../appWindow/SessionManager.h"
#include "BookDetailDialog.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QEvent>

FavoriteBooksWindow::FavoriteBooksWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FavoriteBooksWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &FavoriteBooksWindow::handleResponse);
}

FavoriteBooksWindow::~FavoriteBooksWindow()
{
    delete ui;
}
void FavoriteBooksWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    loadFavoriteBooks();
}
void FavoriteBooksWindow::loadFavoriteBooks()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetFavoriteBooks, params);
    m_networkManager->sendRequest(request);
}
void FavoriteBooksWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() != CommandType::GetFavoriteBooks) return;

    if (response.isSuccess()) {
        QVariantMap data = response.getData();
        m_allFavoriteBooks = data["books"].toList();
        displayBooks(m_allFavoriteBooks);
    } else {
        QMessageBox::critical(this, "Error", "Failed to load favorites: " + response.getMessage());
    }
}

void FavoriteBooksWindow::displayBooks(const QVariantList& books)
{
    clearGrid();

    if (books.isEmpty()) {
        QLabel* emptyLabel = new QLabel("No books found in your favorites.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 18px; color: gray;");
        ui->favoritesGrid->addWidget(emptyLabel, 0, 0, 1, 4);
        return;
    }

    int columns = 4;
    for (int i = 0; i < books.size(); ++i) {
        QVariantMap book = books[i].toMap();
        int row = i / columns;
        int col = i % columns;
        QWidget* bookWidget = new QWidget();
        QVBoxLayout* boxLayout = new QVBoxLayout(bookWidget);
        boxLayout->setContentsMargins(5, 5, 5, 5);
        boxLayout->setAlignment(Qt::AlignCenter);
        QLabel* coverLabel = new QLabel();
        coverLabel->setFixedSize(140, 200);
        coverLabel->setScaledContents(true);
        coverLabel->setCursor(Qt::PointingHandCursor);
        coverLabel->setStyleSheet("border: 2px solid black; border-radius: 8px; background-color: #f0f0f0;");
        coverLabel->installEventFilter(this);
        coverLabel->setProperty("bookData", book);

        QString coverPath = book["coverPath"].toString();
        if (!coverPath.isEmpty()) {
            QPixmap pix(coverPath);
            if (!pix.isNull()) {
                coverLabel->setPixmap(pix.scaled(140, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                coverLabel->setText("No Cover");
                coverLabel->setAlignment(Qt::AlignCenter);
            }
        } else {
            coverLabel->setText("No Cover");
            coverLabel->setAlignment(Qt::AlignCenter);
        }
        QLabel* textLabel = new QLabel();
        QString shortTitle = book["title"].toString();
        if (shortTitle.length() > 18) shortTitle = shortTitle.left(15) + "...";

        textLabel->setText(QString("<b>%1</b><br><font color='#555'>%2</font>")
                               .arg(shortTitle, book["author"].toString()));
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setStyleSheet("font-size: 13px;");
        textLabel->setWordWrap(true);
        boxLayout->addWidget(coverLabel);
        boxLayout->addWidget(textLabel);
        ui->favoritesGrid->addWidget(bookWidget, row, col);
    }
}
void FavoriteBooksWindow::on_searchInput_textChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        displayBooks(m_allFavoriteBooks);
        return;
    }

    QVariantList filteredBooks;
    for (const QVariant& var : m_allFavoriteBooks) {
        QVariantMap book = var.toMap();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        if (title.contains(text, Qt::CaseInsensitive) ||
            author.contains(text, Qt::CaseInsensitive)) {
            filteredBooks.append(book);
        }
    }
    displayBooks(filteredBooks);
}

bool FavoriteBooksWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* label = qobject_cast<QLabel*>(watched);
        if (label && label->property("bookData").isValid()) {
            QVariantMap bookData = label->property("bookData").toMap();
            BookDetailDialog dialog(m_networkManager, bookData, this);
            dialog.exec();
            loadFavoriteBooks();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void FavoriteBooksWindow::clearGrid()
{
    QLayoutItem *item;
    while ((item = ui->favoritesGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void FavoriteBooksWindow::on_backButton_clicked()
{
    emit backButtonClicked();
}