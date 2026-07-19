
#include "genrebrowserwindow.h"
#include "Users/ui_genrebrowserwindow.h"

#include "../appWindow/SessionManager.h"
#include "BookDetailDialog.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QEvent>

GenreBrowserWindow::GenreBrowserWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GenreBrowserWindow),
    m_networkManager(networkManager),
    m_currentMode(GenresMode)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &GenreBrowserWindow::handleResponse);
}

GenreBrowserWindow::~GenreBrowserWindow()
{
    delete ui;
}

void GenreBrowserWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    m_currentMode = GenresMode;
    ui->titleLabel->setText("📚 Browse by Genre");
    ui->searchInput->setPlaceholderText("Search genres...");
    ui->searchInput->clear();
    requestAllGenres();
}
void GenreBrowserWindow::requestAllGenres()
{
    Request request(CommandType::GetAllGenres, QVariantMap());
    m_networkManager->sendRequest(request);
}

// ۲. درخواست لیست کتاب‌های یک ژانر خاص
void GenreBrowserWindow::requestBooksByGenre(const QString& genreName)
{
    QVariantMap params;
    params["genre"] = genreName;
    params["userId"] = SessionManager::instance()->getUserId();

    Request request(CommandType::GetBooksByGenre, params);
    m_networkManager->sendRequest(request);
}

void GenreBrowserWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::GetAllGenres) {
        if (response.isSuccess()) {
            m_allGenres = response.getData()["genres"].toList();
            if (m_currentMode == GenresMode) {
                displayGenres(m_allGenres);
            }
        } else {
            QMessageBox::critical(this, "Error", "Failed to load genres.");
        }
    }
    else if (response.getCommandType() == CommandType::GetBooksByGenre) {
        if (response.isSuccess()) {
            m_currentBooks = response.getData()["books"].toList();
            if (m_currentMode == BooksMode) {
                displayBooks(m_currentBooks);
            }
        } else {
            QMessageBox::critical(this, "Error", "Failed to load books for this genre.");
        }
    }
}
void GenreBrowserWindow::displayGenres(const QVariantList& genresList)
{
    clearGrid();

    int columns = 4;
    for (int i = 0; i < genresList.size(); ++i) {
        QString genreName = genresList[i].toString();
        int row = i / columns;
        int col = i % columns;

        QLabel* genreCard = new QLabel(genreName);
        genreCard->setAlignment(Qt::AlignCenter);
        genreCard->setCursor(Qt::PointingHandCursor);
        genreCard->setStyleSheet(
            "QLabel {"
            "  background-color: #f8f9fa;"
            "  border: 3px solid black;"
            "  border-radius: 15px;"
            "  font: 700 16pt 'Script MT Bold';"
            "  color: black;"
            "  padding: 20px;"
            "}"
            "QLabel:hover {"
            "  background-color: #e2e6ea;"
            "}"
            );
        genreCard->setFixedSize(280, 100);
        genreCard->setProperty("genreName", genreName);
        genreCard->installEventFilter(this);

        ui->genresGrid->addWidget(genreCard, row, col);
    }
}

void GenreBrowserWindow::displayBooks(const QVariantList& booksList)
{
    clearGrid();

    if (booksList.isEmpty()) {
        QLabel* noBookLabel = new QLabel("No active books available in this genre.");
        noBookLabel->setAlignment(Qt::AlignCenter);
        noBookLabel->setStyleSheet("font-size: 18px; color: gray; font-weight: bold;");
        ui->genresGrid->addWidget(noBookLabel, 0, 0, 1, 4);
        return;
    }

    int columns = 5;
    for (int i = 0; i < booksList.size(); ++i) {
        QVariantMap book = booksList[i].toMap();
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
        coverLabel->setStyleSheet("border: 2px solid black; border-radius: 8px; background-color: #eceff1;");

        coverLabel->setProperty("bookData", book);
        coverLabel->installEventFilter(this);
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
        QString title = book["title"].toString();
        if (title.length() > 16) title = title.left(13) + "...";

        textLabel->setText(QString("<b>%1</b><br><font color='#666'>%2</font>").arg(title, book["author"].toString()));
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setStyleSheet("font-size: 12px;");

        boxLayout->addWidget(coverLabel);
        boxLayout->addWidget(textLabel);

        ui->genresGrid->addWidget(bookWidget, row, col);
    }
}
void GenreBrowserWindow::on_searchInput_textChanged(const QString &text)
{
    if (m_currentMode == GenresMode) {
        if (text.trimmed().isEmpty()) {
            displayGenres(m_allGenres);
            return;
        }
        QVariantList filtered;
        for (const QVariant& g : m_allGenres) {
            if (g.toString().contains(text, Qt::CaseInsensitive)) {
                filtered.append(g);
            }
        }
        displayGenres(filtered);
    }
    else if (m_currentMode == BooksMode) {
        if (text.trimmed().isEmpty()) {
            displayBooks(m_currentBooks);
            return;
        }
        QVariantList filtered;
        for (const QVariant& b : m_currentBooks) {
            QVariantMap book = b.toMap();
            if (book["title"].toString().contains(text, Qt::CaseInsensitive) ||
                book["author"].toString().contains(text, Qt::CaseInsensitive)) {
                filtered.append(book);
            }
        }
        displayBooks(filtered);
    }
}

// ۷. فیلتر کردن رویداد کلیک‌ها (ژانر یا کتاب)
bool GenreBrowserWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* label = qobject_cast<QLabel*>(watched);
        if (!label) return false;

        // سناریو اول: کاربر روی یک ژانر کلیک کرده است
        if (label->property("genreName").isValid()) {
            m_selectedGenre = label->property("genreName").toString();
            m_currentMode = BooksMode;

            ui->titleLabel->setText("📚 Genre: " + m_selectedGenre);
            ui->searchInput->setPlaceholderText("Search books in this genre...");
            ui->searchInput->clear();

            requestBooksByGenre(m_selectedGenre);
            return true;
        }
        if (label->property("bookData").isValid()) {
            QVariantMap bookData = label->property("bookData").toMap();
            BookDetailDialog dialog(m_networkManager, bookData, this);
            dialog.exec();
            if (m_currentMode == BooksMode) {
                requestBooksByGenre(m_selectedGenre);
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
void GenreBrowserWindow::on_backButton_clicked()
{
    if (m_currentMode == BooksMode) {
        m_currentMode = GenresMode;
        ui->titleLabel->setText("📚 Browse by Genre");
        ui->searchInput->setPlaceholderText("Search genres...");
        ui->searchInput->clear();
        displayGenres(m_allGenres);
    } else {
        emit backButtonClicked();
    }
}

void GenreBrowserWindow::clearGrid()
{
    QLayoutItem *item;
    while ((item = ui->genresGrid->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}