
#include "mylibrarywindow.h"
#include "Users/ui_mylibrarywindow.h"
#include "../appWindow/SessionManager.h"
#include "../Users/BookDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QTimer>

MyLibraryWindow::MyLibraryWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyLibraryWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    // تنظیمات جایگاه لیبل تعداد کتاب‌ها
    ui->totalBooksLabel->setGeometry(10, 5, 280, 61);

    // اتصال سیگنال دریافت پاسخ از شبکه
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &MyLibraryWindow::handleResponse);

    // اتصال جستجوی زنده
    connect(ui->searchInput, &QLineEdit::textChanged,
            this, &MyLibraryWindow::onSearchTextChanged);
}

MyLibraryWindow::~MyLibraryWindow()
{
    delete ui;
}

// 🟢 هنگام باز شدن صفحه، کتاب‌ها لود می‌شوند
void MyLibraryWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    loadLibrary();
}

// 🟢 ارسال درخواست دریافت کتابخانه کاربر به سرور
void MyLibraryWindow::loadLibrary()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;

    // ارسال کامند دریافت کتابخانه کاربر
    Request request(CommandType::GetUserLibrary, params);
    m_networkManager->sendRequest(request);
}

// 🟢 پردازش پاسخ‌های دریافتی از سرور
void MyLibraryWindow::handleResponse(const Response& response)
{
    // ۱. دریافت لیست کتاب‌های کاربر
    if (response.getCommandType() == CommandType::GetUserLibrary) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            m_allBooks = data["books"].toList();
            displayBooks(m_allBooks);
        } else {
            QMessageBox::warning(this, "Error", "Failed to load library: " + response.getMessage());
        }
    }
    // ۲. دریافت اطلاعات یک کتاب برای نمایش در دیالوگ
    else if (response.getCommandType() == CommandType::GetBookById) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            QVariantMap bookData = resData.contains("book") ? resData["book"].toMap() : resData;
            bookData["userId"] = SessionManager::instance()->getUserId();

            QTimer::singleShot(0, this, [this, bookData]() {
                BookDetailDialog dialog(m_networkManager, bookData, this);
                dialog.exec();
            });
        } else {
            QMessageBox::warning(this, "Error", "Failed to fetch book details: " + response.getMessage());
        }
    }
}

// 🟢 نمایش کارت‌های کتاب در اسکرول آریا
void MyLibraryWindow::displayBooks(const QVariantList& books)
{
    clearLayout();

    // به‌روزرسانی تعداد کل کتاب‌ها در لیبل
    ui->totalBooksLabel->setText(QString("Total Books: %1").arg(books.size()));

    if (books.isEmpty()) {
        QLabel* emptyLabel = new QLabel("No books found in your library.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 22px; color: gray; font-weight: bold; margin-top: 50px;");
        ui->booksLayout->addWidget(emptyLabel);
        return;
    }

    for (const QVariant& var : books) {
        QVariantMap book = var.toMap();

        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString genre = book["genre"].toString();

        // فریم یا کارت اختصاصی هر کتاب
        QFrame* bookCard = new QFrame();
        bookCard->setStyleSheet(
            "QFrame {"
            "  background-color: #ffffff;"
            "  border: 2px solid #2b2b2b;"
            "  border-radius: 10px;"
            "}"
            "QFrame:hover {"
            "  background-color: #e8f5e9;"
            "  border: 2px solid #2e7d32;"
            "}"
            );
        bookCard->setCursor(Qt::PointingHandCursor);
        bookCard->setProperty("bookId", bookId);
        bookCard->installEventFilter(this); // قابلیت کلیک روی کارت

        QHBoxLayout* cardLayout = new QHBoxLayout(bookCard);
        cardLayout->setContentsMargins(20, 12, 20, 12);

        // آیکون و مشخصات کتاب
        QLabel* infoLabel = new QLabel(
            QString("📖 <span style='font-size:18px;'><b>%1</b></span> &nbsp;&nbsp;|&nbsp;&nbsp; ✍️ Author: <b>%2</b> &nbsp;&nbsp;|&nbsp;&nbsp; 🏷️ Genre: <b>%3</b>")
                .arg(title.isEmpty() ? QString("Book #%1").arg(bookId) : title)
                .arg(author.isEmpty() ? "Unknown" : author)
                .arg(genre.isEmpty() ? "General" : genre)
            );
        infoLabel->setStyleSheet("font-size: 16px; color: black; border: none; background: transparent;");

        QLabel* actionHint = new QLabel("Click to View Details ➔");
        actionHint->setStyleSheet("font-size: 14px; color: #2e7d32; font-weight: bold; border: none; background: transparent;");

        cardLayout->addWidget(infoLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(actionHint);

        ui->booksLayout->addWidget(bookCard);
    }

    ui->booksLayout->addStretch();
}

// 🟢 جستجوی زنده بر اساس ژانر یا عنوان کتاب
void MyLibraryWindow::onSearchTextChanged(const QString &text)
{
    filterBooks(text.trimmed());
}

void MyLibraryWindow::filterBooks(const QString& query)
{
    if (query.isEmpty()) {
        displayBooks(m_allBooks);
        return;
    }

    QVariantList filtered;
    for (const QVariant& var : m_allBooks) {
        QVariantMap book = var.toMap();
        QString title = book["title"].toString();
        QString genre = book["genre"].toString();
        QString author = book["author"].toString();

        if (title.contains(query, Qt::CaseInsensitive) ||
            genre.contains(query, Qt::CaseInsensitive) ||
            author.contains(query, Qt::CaseInsensitive)) {
            filtered.append(book);
        }
    }

    displayBooks(filtered);
}

// 🟢 مدیریت کلیک روی کارت کتاب برای باز کردن دیالوگ جزئیات
bool MyLibraryWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QFrame* frame = qobject_cast<QFrame*>(watched);
        if (frame && frame->property("bookId").isValid()) {
            int bookId = frame->property("bookId").toInt();

            QVariantMap params;
            params["bookId"] = bookId;
            params["userId"] = SessionManager::instance()->getUserId();

            Request request(CommandType::GetBookById, params);
            m_networkManager->sendRequest(request);
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MyLibraryWindow::clearLayout()
{
    QLayoutItem *item;
    while ((item = ui->booksLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void MyLibraryWindow::on_backButton_clicked()
{
    emit backButtonClicked();
}