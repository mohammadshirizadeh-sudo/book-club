#include "userwindow.h"
#include "appWindow/ui_userwindow.h"
#include "SessionManager.h"
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>

UserWindow::UserWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);


    ui->freeBooksListWidget->setViewMode(QListView::IconMode);
    ui->freeBooksListWidget->setResizeMode(QListView::Adjust);
    ui->freeBooksListWidget->setMovement(QListView::Static);
    ui->freeBooksListWidget->setIconSize(QSize(250, 380));



    ui->recommendedBooksListWidget->setViewMode(QListView::IconMode);
    ui->recommendedBooksListWidget->setResizeMode(QListView::Adjust);
    ui->recommendedBooksListWidget->setMovement(QListView::Static);
    // چون فضای بزرگتری دارید و ۲ کتاب جا می‌گیرد، ابعاد ۲۲۰ در ۳۲۰ انتخاب مناسبی است
    ui->recommendedBooksListWidget->setIconSize(QSize(300, 380));



    //for recently added books
    ui->newBooksListWidget->setViewMode(QListView::IconMode);
    ui->newBooksListWidget->setResizeMode(QListView::Adjust);
    ui->newBooksListWidget->setMovement(QListView::Static);

    // تنظیم ظرفیت فضای آیکون‌ها (مثلاً عرض 250 و ارتفاع 420)
    ui->newBooksListWidget->setIconSize(QSize(250, 420));





    ui->newBooksListWidget->setGridSize(QSize(260, 480));
    ui->freeBooksListWidget->setGridSize(QSize(260, 480));
    ui->recommendedBooksListWidget->setGridSize(QSize(260, 480));


    // اتصال به پاسخ سرور
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &UserWindow::handleResponse);

}

UserWindow::~UserWindow()
{
    delete ui;
}

void UserWindow::loadFreeBooks()
{
    // درخواست کتاب‌های رایگان

    qDebug() << "📚 [Client] Sending GetFreeBooks request to server...";
    Request request(CommandType::GetFreeBooks, {});
    m_networkManager->sendRequest(request);
}


/*
void UserWindow::handleResponse(const Response& response)
{
    qDebug()<<"we enter at handleresponse";
    if (response.getCommandType() != CommandType::GetFreeBooks) {

        return;
    }
    qDebug()<<"we quit from handleresponse";

    // if (!response.isSuccess()) {
    //     ui->statusLabel->setText("❌ Failed to load books: " + response.getMessage());
    //     return;
    // }

    QVariantList books = response.getData()["books"].toList();
    int count = response.getData()["count"].toInt();

    ui->freeBooksListWidget->clear();
    m_booksCache.clear();

    for (const QVariant& bookVar : books) {
        QVariantMap book = bookVar.toMap();
        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString coverPath = book["coverPath"].toString();

        // ذخیره در Cache
        m_booksCache[bookId] = book;

        // ایجاد آیتم
        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId);

        // متن آیتم (عنوان + نویسنده)
        QString displayText = title + "\n" + author;
        item->setText(displayText);

        // تنظیم عکس (اگر وجود دارد)
        if (!coverPath.isEmpty()) {
            QPixmap pixmap(coverPath);
            if (!pixmap.isNull()) {
                // مقیاس کردن عکس به اندازه 100x150
                QPixmap scaled = pixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                item->setIcon(QIcon(scaled));
            }
        }

        ui->freeBooksListWidget->addItem(item);
    }
    // ui->statusLabel->setText(QString("✅ %1 free books loaded").arg(count));
}
*/


void UserWindow::handleResponse(const Response& response)
{
    // ۱. مدیریت پاسخ کتاب‌های رایگان
    if (response.getCommandType() == CommandType::GetFreeBooks) {
        m_allFreeBooks = response.getData()["books"].toList();
        m_currentPage = 0;
        updateBooksDisplay();
        return;
    }

    // 💡 ۲. مدیریت پاسخ کتاب‌های پیشنهادی
    if (response.getCommandType() == CommandType::GetRecommendedBooks) {
        m_allRecBooks = response.getData()["books"].toList();
        m_currentRecPage = 0; // ریست به صفحه اول
        updateRecommendedBooksDisplay();
        return;
    }



    if (response.getCommandType() == CommandType::GetNewBooks) {
        m_allNewBooks = response.getData()["books"].toList();
        m_currentNewPage = 0; // ریست به صفحه اول
        updateNewBooksDisplay();
        return;
    }
}
void UserWindow::updateBooksDisplay()
{
    ui->freeBooksListWidget->clear();
    m_booksCache.clear();

    if (m_allFreeBooks.isEmpty()) {
        return;
    }

    int startIndex = m_currentPage * m_booksPerPage;
    int endIndex = qMin(startIndex + m_booksPerPage, m_allFreeBooks.size());

    for (int i = startIndex; i < endIndex; ++i) {
        QVariantMap book = m_allFreeBooks[i].toMap();
        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString coverPath = book["coverPath"].toString();

        m_booksCache[bookId] = book;

        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId);

        // متن زیر کتاب (می‌تونی فونتش رو هم بزرگتر کنی)
        item->setText(title + "\n" + author);
        item->setTextAlignment(Qt::AlignCenter);

        if (!coverPath.isEmpty()) {
            QPixmap pixmap(coverPath);
            if (!pixmap.isNull()) {
                // 🖼️ عکس رو بزرگتر اسکیل می‌کنیم تا تمام فضای اختصاصی رو پر کنه
                QPixmap scaled = pixmap.scaled(240, 360, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                item->setIcon(QIcon(scaled));
            }
        }
        ui->freeBooksListWidget->addItem(item);
    }

    // مدیریت دکمه‌ها بدون تغییر باقی می‌مونه و عالی کار می‌کنه
    ui->nextPushButton->setEnabled(endIndex < m_allFreeBooks.size());
    ui->prevPushButton->setEnabled(m_currentPage > 0);
}


void UserWindow::on_freeBooksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int bookId = item->data(Qt::UserRole).toInt();

    if (m_booksCache.contains(bookId)) {
        QVariantMap book = m_booksCache[bookId];

        // نمایش اطلاعات کتاب (می‌توانید در یک Label یا پنجره جدا)
        QString info = QString(
                           "Title: %1\n"
                           "Author: %2\n"
                           "Genre: %3\n"
                           "Price: $%4\n"
                           "Rating: %5"
                           ).arg(book["title"].toString())
                           .arg(book["author"].toString())
                           .arg(book["genre"].toString())
                           .arg(book["finalPrice"].toDouble())
                           .arg(book["averageRating"].toDouble());

        QMessageBox::information(this, "Book Details", info);
    }
}
void UserWindow::on_nextPushButton_clicked()
{

    m_currentPage++;
    updateBooksDisplay();

}


void UserWindow::on_prevPushButton_clicked()
{
    if (m_currentPage > 0) {
        m_currentPage--;
        updateBooksDisplay();
    }
}


void UserWindow::loadRecommendedBooks()
{
    qDebug() << "📚 [Client] Sending GetRecommendedBooks request to server...";

    // 🔑 دریافت آیدی کاربر فعلی از SessionManager
    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["userId"] = userId;
    params["limit"] = 20; // مثلاً سرور تا ۲۰ کتاب پیشنهادی پیدا کند و بیاورد

    Request request(CommandType::GetRecommendedBooks, params);
    m_networkManager->sendRequest(request);
}



void UserWindow::updateRecommendedBooksDisplay()
{
    ui->recommendedBooksListWidget->clear();
    m_recBooksCache.clear();

    if (m_allRecBooks.isEmpty()) {
        return;
    }

    // محاسبه ایندکس شروع و پایان بر اساس ظرفیت ۲
    int startIndex = m_currentRecPage * m_recBooksPerPage;
    int endIndex = qMin(startIndex + m_recBooksPerPage, m_allRecBooks.size());

    for (int i = startIndex; i < endIndex; ++i) {
        QVariantMap book = m_allRecBooks[i].toMap();
        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString coverPath = book["coverPath"].toString();

        m_recBooksCache[bookId] = book;

        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId);

        item->setText(title + "\n" + author);
        item->setTextAlignment(Qt::AlignCenter);

        if (!coverPath.isEmpty()) {
            QPixmap pixmap(coverPath);
            if (!pixmap.isNull()) {
                // سایز عکس کمی کوچکتر از باکس آیکون (مثلا ۲۱۰ در ۳۰۰) تا کنار هم قشنگ جا شوند
                QPixmap scaled = pixmap.scaled(280, 300, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                item->setIcon(QIcon(scaled));
            }
        }
        ui->recommendedBooksListWidget->addItem(item);
    }

    // مدیریت وضعیت فعال/غیرفعال بودن دکمه‌های بعدی و قبلیِ پیشنهادی‌ها
    ui->nextRecPushButton->setEnabled(endIndex < m_allRecBooks.size());
    ui->prevRecPushButton->setEnabled(m_currentRecPage > 0);
}


void UserWindow::on_nextRecPushButton_clicked()
{
    m_currentRecPage++;
    updateRecommendedBooksDisplay();
}

void UserWindow::on_prevRecPushButton_clicked()
{
    if (m_currentRecPage > 0) {
        m_currentRecPage--;
        updateRecommendedBooksDisplay();
    }
}

void UserWindow::on_recommendedBooksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int bookId = item->data(Qt::UserRole).toInt();

    if (m_recBooksCache.contains(bookId)) {
        QVariantMap book = m_recBooksCache[bookId];

        QString info = QString(
                           "Title: %1\n"
                           "Author: %2\n"
                           "Price: $%3\n"
                           "Final Price: $%4\n"
                           "Rating: %5"
                           ).arg(book["title"].toString())
                           .arg(book["author"].toString())
                           .arg(book["price"].toDouble())
                           .arg(book["finalPrice"].toDouble())
                           .arg(book["averageRating"].toDouble());

        QMessageBox::information(this, "Recommended Book Details", info);
    }
}


//for recently added books
void UserWindow::loadNewBooks()
{
    qDebug() << "📚 [Client] Sending GetNewBooks request to server...";

    QVariantMap params;
    params["limit"] = 20; // تعداد کل کتاب‌های جدیدی که سرور بازمی‌گرداند

    Request request(CommandType::GetNewBooks, params);
    m_networkManager->sendRequest(request);
}





void UserWindow::updateNewBooksDisplay()
{
    ui->newBooksListWidget->clear();
    m_newBooksCache.clear();

    if (m_allNewBooks.isEmpty()) {
        return;
    }

    // محاسبه ایندکس شروع و پایان بر اساس مروارید صفحات
    int startIndex = m_currentNewPage * m_newBooksPerPage;
    int endIndex = qMin(startIndex + m_newBooksPerPage, m_allNewBooks.size());

    for (int i = startIndex; i < endIndex; ++i) {
        QVariantMap book = m_allNewBooks[i].toMap();
        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString coverPath = book["coverPath"].toString();

        m_newBooksCache[bookId] = book;

        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId);

        // تنظیم متن زیر عکس کتاب
        item->setText(title + "\n" + author);
        item->setTextAlignment(Qt::AlignCenter);

        if (!coverPath.isEmpty()) {
            QPixmap pixmap(coverPath);
            if (!pixmap.isNull()) {
                // اسکیل کردن عکس متناسب با ابعادی که در نظر دارید (مثلا 240 در 400)
                QPixmap scaled = pixmap.scaled(240, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                item->setIcon(QIcon(scaled));
            }
        }
        ui->newBooksListWidget->addItem(item);
    }

    // فعال یا غیرفعال کردن دکمه‌های بعدی و قبلی مخصوص کتاب‌های جدید
    ui->nextNewPushButton->setEnabled(endIndex < m_allNewBooks.size());
    ui->prevNewPushButton->setEnabled(m_currentNewPage > 0);
}

void UserWindow::on_nextNewPushButton_clicked()
{
    m_currentNewPage++;
    updateNewBooksDisplay();
}

void UserWindow::on_prevNewPushButton_clicked()
{
    if (m_currentNewPage > 0) {
        m_currentNewPage--;
        updateNewBooksDisplay();
    }
}

void UserWindow::on_newBooksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int bookId = item->data(Qt::UserRole).toInt();

    if (m_newBooksCache.contains(bookId)) {
        QVariantMap book = m_newBooksCache[bookId];

        QString info = QString(
                           "Title: %1\n"
                           "Author: %2\n"
                           "Price: $%3\n"
                           "Final Price: $%4\n"
                           "Rating: %5"
                           ).arg(book["title"].toString())
                           .arg(book["author"].toString())
                           .arg(book["price"].toDouble())
                           .arg(book["finalPrice"].toDouble())
                           .arg(book["averageRating"].toDouble());

        QMessageBox::information(this, "Book Details", info);
    }
}


void UserWindow::on_pushButton_2_clicked()
{
    emit userProfileWindow();
}


void UserWindow::on_pushButton_5_clicked()
{
    emit searchWindow();

}

