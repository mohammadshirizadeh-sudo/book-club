#include "mylibrarywindow.h"
#include "Users/ui_mylibrarywindow.h"
#include "../appWindow/SessionManager.h"
#include "../Users/BookDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QInputDialog>

MyLibraryWindow::MyLibraryWindow(NetworkManager* networkManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyLibraryWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);
    qDebug() << "MyLibraryWindow constructed:" << this;

    // تنظیمات جایگاه لیبل تعداد کتاب‌ها
    ui->totalBooksLabel->setGeometry(10, 5, 280, 61);

    connect(m_networkManager,
            &NetworkManager::responseReceived,
            this,
            &MyLibraryWindow::handleResponse,
            Qt::UniqueConnection);

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

    Request request(CommandType::GetUserLibrary, params);
    m_networkManager->sendRequest(request);
}

// 🟢 پردازش پاسخ‌های دریافتی از سرور
void MyLibraryWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // ۱. دریافت لیست کتاب‌های کاربر
    if (type == CommandType::GetUserLibrary) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            m_allBooks = data["books"].toList();
            displayBooks(m_allBooks);
        } else {
            QMessageBox::warning(this, "Error", "Failed to load library: " + response.getMessage());
        }
    }
    // ۲. دریافت اطلاعات یک کتاب برای نمایش در دیالوگ
    else if (type == CommandType::GetBookById) {
        if (!m_isFetchingBook)
            return;
        m_isFetchingBook = false;

        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            QVariantMap bookData = resData.contains("book") ? resData["book"].toMap() : resData;
            bookData["userId"] = SessionManager::instance()->getUserId();

            QTimer::singleShot(0, this, [this, bookData]() {
                BookDetailDialog dialog(m_networkManager, bookData, this);
                dialog.exec();
                m_isFetchingBook = false;
            });
        } else {
            QMessageBox::warning(this, "Error", "Failed to fetch book details: " + response.getMessage());
        }
    }
    // ۳. دریافت لیست قفسه‌ها (جهت افزودن کتاب به قفسه)
    else if (type == CommandType::GetUserShelves) {
        if (m_selectedBookIdForShelf <= 0) return;

        if (response.isSuccess()) {
            QVariantList shelves = response.getData()["shelves"].toList();

            // اگر قفسه جدیدی ایجاد شده بود، ID آن را پیدا کرده و کتاب را اضافه می‌کنیم
            if (!m_newShelfNameToAdd.isEmpty()) {
                int newShelfId = -1;
                for (const QVariant& var : shelves) {
                    QVariantMap s = var.toMap();
                    if (s["name"].toString() == m_newShelfNameToAdd) {
                        newShelfId = s["shelfId"].toInt();
                        break;
                    }
                }

                if (newShelfId > 0) {
                    int userId = SessionManager::instance()->getUserId();
                    QVariantMap params;
                    params["userId"] = userId;
                    params["shelfId"] = newShelfId;
                    params["bookId"] = m_selectedBookIdForShelf;

                    Request req(CommandType::AddBookToShelf, params);
                    m_networkManager->sendRequest(req);
                } else {
                    QMessageBox::warning(this, "Error", "Could not find newly created shelf.");
                    m_selectedBookIdForShelf = -1;
                }
                m_newShelfNameToAdd = "";
            } else {
                // نمایش دیالوگ انتخاب قفسه به کاربر
                showShelfSelectionDialog(shelves);
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to load shelves: " + response.getMessage());
            m_selectedBookIdForShelf = -1;
        }
    }
    // ۴. پاسخ ایجاد قفسه جدید
    else if (type == CommandType::CreateShelf) {
        if (response.isSuccess()) {
            // پس از ساخت موفق قفسه، مجدداً لیست قفسه‌ها را می‌گیریم تا ID قفسه جدید مشخص شود
            int userId = SessionManager::instance()->getUserId();
            QVariantMap params;
            params["userId"] = userId;
            Request req(CommandType::GetUserShelves, params);
            m_networkManager->sendRequest(req);
        } else {
            QMessageBox::warning(this, "Error", response.getMessage());
            m_selectedBookIdForShelf = -1;
            m_newShelfNameToAdd = "";
        }
    }
    // ۵. پاسخ اضافه شدن کتاب به قفسه
    else if (type == CommandType::AddBookToShelf) {
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Book successfully added to shelf!");
        } else {
            QMessageBox::warning(this, "Error", response.getMessage());
        }
        m_selectedBookIdForShelf = -1;
    }
}

// 🟢 نمایش کارت‌های کتاب در اسکرول آریا
void MyLibraryWindow::displayBooks(const QVariantList& books)
{
    clearLayout();

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
        bookCard->installEventFilter(this);

        QHBoxLayout* cardLayout = new QHBoxLayout(bookCard);
        cardLayout->setContentsMargins(20, 12, 20, 12);

        QLabel* infoLabel = new QLabel(
            QString("📖 <span style='font-size:18px;'><b>%1</b></span> &nbsp;&nbsp;|&nbsp;&nbsp; ✍️ Author: <b>%2</b> &nbsp;&nbsp;|&nbsp;&nbsp; 🏷️ Genre: <b>%3</b>")
                .arg(title.isEmpty() ? QString("Book #%1").arg(bookId) : title)
                .arg(author.isEmpty() ? "Unknown" : author)
                .arg(genre.isEmpty() ? "General" : genre)
            );
        infoLabel->setStyleSheet("font-size: 16px; color: black; border: none; background: transparent;");

        // 🟢 دکمه Add to Shelf
        QPushButton* addToShelfBtn = new QPushButton("📚 Add to Shelf");
        addToShelfBtn->setCursor(Qt::PointingHandCursor);
        addToShelfBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #2e7d32;"
            "  color: white;"
            "  font-weight: bold;"
            "  font-size: 13px;"
            "  padding: 6px 12px;"
            "  border-radius: 6px;"
            "  border: none;"
            "}"
            "QPushButton:hover {"
            "  background-color: #1b5e20;"
            "}"
            );

        // کلیک روی دکمه Add to Shelf
        connect(addToShelfBtn, &QPushButton::clicked, this, [this, bookId]() {
            m_selectedBookIdForShelf = bookId;
            int userId = SessionManager::instance()->getUserId();

            QVariantMap params;
            params["userId"] = userId;
            Request request(CommandType::GetUserShelves, params);
            m_networkManager->sendRequest(request);
        });

        cardLayout->addWidget(infoLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(addToShelfBtn);

        ui->booksLayout->addWidget(bookCard);
    }

    ui->booksLayout->addStretch();
}

// 🟢 دیالوگ انتخاب قفسه یا ایجاد قفسه جدید
void MyLibraryWindow::showShelfSelectionDialog(const QVariantList& shelves)
{
    QStringList options;
    options.append("➕ Create New Shelf..."); // گزینه اول برای ساخت قفسه جدید

    QMap<QString, int> shelfNameToIdMap;
    for (const QVariant& var : shelves) {
        QVariantMap shelf = var.toMap();
        QString name = shelf["name"].toString();
        int shelfId = shelf["shelfId"].toInt();

        options.append(name);
        shelfNameToIdMap[name] = shelfId;
    }

    bool ok;
    QString selectedOption = QInputDialog::getItem(this, "Add to Shelf",
                                                   "Select a shelf or create new:",
                                                   options, 0, false, &ok);

    if (ok && !selectedOption.isEmpty()) {
        int userId = SessionManager::instance()->getUserId();

        // اگر کاربر گزینه ایجاد قفسه جدید را انتخاب کرد
        if (selectedOption == "➕ Create New Shelf...") {
            bool createOk;
            QString newShelfName = QInputDialog::getText(this, "New Shelf",
                                                         "Enter new shelf name:",
                                                         QLineEdit::Normal, "", &createOk);
            if (createOk && !newShelfName.trimmed().isEmpty()) {
                m_newShelfNameToAdd = newShelfName.trimmed();

                QVariantMap params;
                params["userId"] = userId;
                params["name"] = m_newShelfNameToAdd;

                Request req(CommandType::CreateShelf, params);
                m_networkManager->sendRequest(req);
            } else {
                m_selectedBookIdForShelf = -1;
            }
        }
        // اگر یکی از قفسه‌های موجود انتخاب شد
        else {
            int shelfId = shelfNameToIdMap[selectedOption];

            QVariantMap params;
            params["userId"] = userId;
            params["shelfId"] = shelfId;
            params["bookId"] = m_selectedBookIdForShelf;

            Request req(CommandType::AddBookToShelf, params);
            m_networkManager->sendRequest(req);
        }
    } else {
        m_selectedBookIdForShelf = -1;
    }
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

bool MyLibraryWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        // اگر روی خود دکمه کلیک شده باشد، اجازه دهیم خود دکمه سیگنال clicked را شلیک کند
        if (qobject_cast<QPushButton*>(watched)) return false;

        if (m_isFetchingBook) return true;

        QWidget* widget = qobject_cast<QWidget*>(watched);
        while (widget && !qobject_cast<QFrame*>(widget)) {
            widget = widget->parentWidget();
        }
        QFrame* frame = qobject_cast<QFrame*>(widget);

        if (frame && frame->property("bookId").isValid()) {
            int bookId = frame->property("bookId").toInt();

            m_isFetchingBook = true;

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