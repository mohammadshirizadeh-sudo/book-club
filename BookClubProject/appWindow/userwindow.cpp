#include "userwindow.h"
#include "appWindow/ui_userwindow.h"
#include "SessionManager.h"
#include "../Users/BookDetailDialog.h"
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QPointer>

#include <QLabel>
#include <QVBoxLayout>

UserWindow::UserWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);


    ui->freeBooksLabel->installEventFilter(this);
    ui->freeBooksLabel->setCursor(Qt::PointingHandCursor);
    ui->freeBooksLabel->setAlignment(Qt::AlignCenter);

    ui->recommendedBooksLabel1->installEventFilter(this);
    ui->recommendedBooksLabel1->setCursor(Qt::PointingHandCursor);
    ui->recommendedBooksLabel1->setAlignment(Qt::AlignCenter);
    ui->recommendedBooksTitleLabel1->setAlignment(Qt::AlignCenter);


    // تنظیمات کتاب دوم پیشنهادی (راست)
    ui->recommendedBooksLabel2->installEventFilter(this);
    ui->recommendedBooksLabel2->setCursor(Qt::PointingHandCursor);
    ui->recommendedBooksLabel2->setAlignment(Qt::AlignCenter);
    ui->recommendedBooksTitleLabel2->setAlignment(Qt::AlignCenter);

    ui->newBooksLabel->installEventFilter(this);
    ui->newBooksLabel->setCursor(Qt::PointingHandCursor);
    ui->newBooksLabel->setAlignment(Qt::AlignCenter);

    // ۲. تراز کردن متن‌های زیر کتاب‌ها در وسط
    ui->freeBooksLabel->setAlignment(Qt::AlignCenter);
    ui->recommendedBooksLabel->setAlignment(Qt::AlignCenter);
    ui->newBooksTitleLabel->setAlignment(Qt::AlignCenter);


    ui->bestSellersLabel->installEventFilter(this);
    ui->bestSellersLabel->setCursor(Qt::PointingHandCursor);
    ui->bestSellersLabel->setAlignment(Qt::AlignCenter);
    ui->bestSellersTitleLabel->setAlignment(Qt::AlignCenter);

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
    QVariantMap data;
    data["userId"] = SessionManager::instance()->getUserId();
    Request request(CommandType::GetFreeBooks, data);


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
    if (response.getCommandType() == CommandType::GetBestSellers) {
        m_allBestSellers = response.getData()["books"].toList();
        m_currentBestSellerPage = 0; // ریست به صفحه اول
        updateBestSellersDisplay();
        return;
    }

    if (response.getCommandType() == CommandType::GetBookCover)
    {
        if (!response.isSuccess())
            return;
        int bookId =response.getData()["bookId"].toInt();
        QByteArray raw = QByteArray::fromBase64(response.getData()["coverData"].toByteArray());
        QPixmap pixmap;
        pixmap.loadFromData(raw);
        if (pixmap.isNull())return;
        m_coverCache[bookId] = pixmap;
        for (QPointer<QLabel> label : m_pendingCoverLabels.values(bookId))
        {
            if (label){
                label->setPixmap(pixmap.scaled(label->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            }
        }
        m_pendingCoverLabels.remove(bookId);
        return;
    }
}
void UserWindow::updateBooksDisplay()
{
    // اگر کتابی نبود، لیبل‌ها را خالی کن
    if (m_allFreeBooks.isEmpty() || m_currentPage < 0 || m_currentPage >= m_allFreeBooks.size()) {
        ui->freeBooksLabel->clear();
        ui->freeBooksTitleLabel->setText("No Books Available");
        return;
    }

    // چون در هر صفحه فقط یک کتاب داریم، ایندکس ما دقیقاً همان شماره صفحه (m_currentPage) است
    QVariantMap book = m_allFreeBooks[m_currentPage].toMap();
    QString title = book["title"].toString();
    QString author = book["author"].toString();
    QString coverPath = book["coverPath"].toString();

    // تنظیم متن عنوان و نویسنده
    ui->freeBooksTitleLabel->setText(title + "\n" + author);

    // تنظیم عکس روی لیبل
    if (!coverPath.isEmpty()) {
        int bookId =book["bookId"].toInt();
        loadCoverInto(ui->freeBooksLabel,bookId,QSize(240,360));
    } else {
        ui->freeBooksLabel->setText("No Cover");
    }
    // مدیریت فعال/غیرفعال بودن دکمه‌ها
    ui->nextPushButton->setEnabled(m_currentPage < m_allFreeBooks.size() - 1);
    ui->prevPushButton->setEnabled(m_currentPage > 0);
}

/*
void UserWindow::on_freeBooksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int bookId = item->data(Qt::UserRole).toInt();

    if (m_booksCache.contains(bookId)) {
        QVariantMap book = m_booksCache[bookId];
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
*/
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
    if (m_allRecBooks.isEmpty()) {
        ui->recommendedBooksLabel1->clear();
        ui->recommendedBooksTitleLabel1->setText("No Recommendations");
        ui->recommendedBooksLabel2->clear();
        ui->recommendedBooksTitleLabel2->clear();
        ui->nextRecPushButton->setEnabled(false);
        ui->prevRecPushButton->setEnabled(false);
        return;
    }

    int booksPerPage = 2;
    int startIndex = m_currentRecPage * booksPerPage;

    // ---------------- کتاب اول (سمت چپ) ----------------
    if (startIndex < m_allRecBooks.size()) {
        QVariantMap book1 = m_allRecBooks[startIndex].toMap();
        QString title1 = book1["title"].toString();
        QString author1 = book1["author"].toString();
        QString coverPath1 = book1["coverPath"].toString();

        ui->recommendedBooksTitleLabel1->setText(title1 + "\n" + author1);
        ui->recommendedBooksTitleLabel1->setVisible(true);
        ui->recommendedBooksLabel1->setVisible(true);

        if (!coverPath1.isEmpty()) {
            int bookId = book1["bookId"].toInt();

            loadCoverInto(
                ui->recommendedBooksLabel1, // 🟢 اصلاح شد: از Label1 استفاده کنید
                bookId,
                QSize(240,360)
                );
        } else {
            ui->recommendedBooksLabel1->setText("No Cover");
        }
    }

    // ---------------- کتاب دوم (سمت راست) ----------------
    if (startIndex + 1 < m_allRecBooks.size()) {
        QVariantMap book2 = m_allRecBooks[startIndex + 1].toMap();
        QString title2 = book2["title"].toString();
        QString author2 = book2["author"].toString();
        QString coverPath2 = book2["coverPath"].toString();

        ui->recommendedBooksTitleLabel2->setText(title2 + "\n" + author2);
        ui->recommendedBooksTitleLabel2->setVisible(true);
        ui->recommendedBooksLabel2->setVisible(true);

        if (!coverPath2.isEmpty()) {
            int bookId = book2["bookId"].toInt();

            loadCoverInto(
                ui->recommendedBooksLabel2, // 🟢 اصلاح شد: از Label2 استفاده کنید
                bookId,
                QSize(240,360)
                );
        } else {
            ui->recommendedBooksLabel2->setText("No Cover");
        }
    } else {
        ui->recommendedBooksLabel2->clear();
        ui->recommendedBooksTitleLabel2->clear();
        ui->recommendedBooksLabel2->setVisible(false);
        ui->recommendedBooksTitleLabel2->setVisible(false);
    }

    ui->nextRecPushButton->setEnabled((m_currentRecPage + 1) * booksPerPage < m_allRecBooks.size());
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

/*
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
*/


//for recently added books
void UserWindow::loadNewBooks()
{
    qDebug() << "📚 [Client] Sending GetNewBooks request to server...";

    QVariantMap params;
    params["limit"] = 20; // تعداد کل کتاب‌های جدیدی که سرور بازمی‌گرداند
    params["userId"] = SessionManager::instance()->getUserId();

    Request request(CommandType::GetNewBooks, params);
    m_networkManager->sendRequest(request);
}




/*
void UserWindow::updateNewBooksDisplay()
{
    ui->newBooksListWidget->clear();
    m_newBooksCache.clear();

    if (m_allNewBooks.isEmpty()) {
        return;
    }

      ui->newBooksListWidget->setItemAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

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
        // item->setTextAlignment(Qt::AlignCenter);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);

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
    ui->newBooksListWidget->setItemAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // فعال یا غیرفعال کردن دکمه‌های بعدی و قبلی مخصوص کتاب‌های جدید
    ui->nextNewPushButton->setEnabled(endIndex < m_allNewBooks.size());
    ui->prevNewPushButton->setEnabled(m_currentNewPage > 0);
}
*/



void UserWindow::updateNewBooksDisplay()
{
    if (m_allNewBooks.isEmpty() || m_currentNewPage < 0 || m_currentNewPage >= m_allNewBooks.size()) {
        ui->newBooksLabel->clear();
        ui->newBooksTitleLabel->setText("No New Books");
        return;
    }

    QVariantMap book = m_allNewBooks[m_currentNewPage].toMap();
    QString title = book["title"].toString();
    QString author = book["author"].toString();
    QString coverPath = book["coverPath"].toString();

    ui->newBooksTitleLabel->setText(title + "\n" + author);

    if (!coverPath.isEmpty()) {
        int bookId =
            book["bookId"].toInt();


        loadCoverInto(
            ui->newBooksLabel,
            bookId,
            QSize(240,360)
            );
    } else {
        ui->newBooksLabel->setText("No Cover");
    }

    ui->nextNewPushButton->setEnabled(m_currentNewPage < m_allNewBooks.size() - 1);
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
/*
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
*/



bool UserWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->freeBooksLabel) {
            onFreeBookClicked();
            return true;
        }
        else if (watched == ui->recommendedBooksLabel1) {
            onRecommendedBookClicked(0); // کتاب اول (سمت چپ)
            return true;
        }
        else if (watched == ui->recommendedBooksLabel2) {
            onRecommendedBookClicked(1); // کتاب دوم (سمت راست)
            return true;
        }
        else if (watched == ui->newBooksLabel) {
            onNewBookClicked();
            return true;
        }
        else if (watched == ui->bestSellersLabel) {
            onBestSellerClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}



void UserWindow::onFreeBookClicked()
{
    if (m_currentPage >= 0 && m_currentPage < m_allFreeBooks.size()) {
        QVariantMap book = m_allFreeBooks[m_currentPage].toMap();
        BookDetailDialog dialog(m_networkManager , book, this);
        dialog.exec();
    }
}

void UserWindow::onRecommendedBookClicked(int offset)
{
    int booksPerPage = 2;
    int targetIndex = (m_currentRecPage * booksPerPage) + offset;

    if (targetIndex >= 0 && targetIndex < m_allRecBooks.size()) {
        QVariantMap book = m_allRecBooks[targetIndex].toMap();
        BookDetailDialog dialog(m_networkManager ,book, this);
        dialog.exec();
    }
}

void UserWindow::onNewBookClicked()
{
    if (m_currentNewPage >= 0 && m_currentNewPage < m_allNewBooks.size()) {
        QVariantMap book = m_allNewBooks[m_currentNewPage].toMap();
        BookDetailDialog dialog(m_networkManager ,book, this);
        dialog.exec();
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


void UserWindow::loadBestSellers()
{
    qDebug() << "📚 [Client] Sending GetBestSellers request to server...";

    QVariantMap params;
    params["limit"] = 20; // تعداد کتاب‌های پرفروشی که سرور بازمی‌گرداند
    params["userId"] = SessionManager::instance()->getUserId();

    Request request(CommandType::GetBestSellers, params);
    m_networkManager->sendRequest(request);
}

void UserWindow::updateBestSellersDisplay()
{
    if (m_allBestSellers.isEmpty() || m_currentBestSellerPage < 0 || m_currentBestSellerPage >= m_allBestSellers.size()) {
        ui->bestSellersLabel->clear();
        ui->bestSellersTitleLabel->setText("No Best Sellers Available");
        return;
    }

    QVariantMap book = m_allBestSellers[m_currentBestSellerPage].toMap();
    QString title = book["title"].toString();
    QString author = book["author"].toString();
    QString coverPath = book["coverPath"].toString();

    // تنظیم متن عنوان و نویسنده
    ui->bestSellersTitleLabel->setText(title + "\n" + author);

    // تنظیم عکس روی لیبل
    if (!coverPath.isEmpty()) {
        int bookId =
            book["bookId"].toInt();


        loadCoverInto(
            ui->bestSellersLabel,
            bookId,
            QSize(240,360)
            );
    } else {
        ui->bestSellersLabel->setText("No Cover");
    }

    // مدیریت فعال/غیرفعال بودن دکمه‌ها
    ui->nextBestSellerPushButton->setEnabled(m_currentBestSellerPage < m_allBestSellers.size() - 1);
    ui->prevBestSellerPushButton->setEnabled(m_currentBestSellerPage > 0);
}


void UserWindow::on_nextBestSellerPushButton_clicked()
{
    m_currentBestSellerPage++;
    updateBestSellersDisplay();
}

void UserWindow::on_prevBestSellerPushButton_clicked()
{
    if (m_currentBestSellerPage > 0) {
        m_currentBestSellerPage--;
        updateBestSellersDisplay();
    }
}


void UserWindow::onBestSellerClicked()
{
    if (m_currentBestSellerPage >= 0 && m_currentBestSellerPage < m_allBestSellers.size()) {
        QVariantMap book = m_allBestSellers[m_currentBestSellerPage].toMap();
        BookDetailDialog dialog(m_networkManager ,book, this);
        dialog.exec();
    }
}


/*
void UserWindow::on_bestSellersListWidget_itemClicked(QListWidgetItem *item)
{

    int index = ui->bestSellersListWidget->row(item);

    if (index >= 0 && index < m_allBestSellers.size()) {
        QVariantMap book = m_allBestSellers[index].toMap();

        // باز کردن پنجره جزئیات کتاب
        BookDetailDialog dialog(book, this);
        dialog.exec();
    }
}
*/




void UserWindow::loadCoverInto(
    QLabel* label,
    int bookId,
    QSize targetSize)
{
    if (m_coverCache.contains(bookId))
    {
        label->setPixmap(
            m_coverCache[bookId]
                .scaled(
                    targetSize,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    )
            );

        return;
    }


    label->setText("Loading...");


    m_pendingCoverLabels.insert(bookId,label);


    m_networkManager->requestBookCover(bookId);
}


void UserWindow::on_pushButton_7_clicked()
{
    emit genrebrowsWindow();

}


void UserWindow::on_pushButton_4_clicked()
{
    emit cartWindow();

}


void UserWindow::on_pushButton_3_clicked()
{
    emit libraryWindow();
}


void UserWindow::on_pushButton_8_clicked()
{
    emit shelfWindow();
}

