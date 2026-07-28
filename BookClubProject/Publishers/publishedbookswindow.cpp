#include "publishedbookswindow.h"
#include "Publishers/ui_publishedbookswindow.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Users/BookDetailDialog.h"
#include "../appWindow/SessionManager.h"

#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QIcon>

PublishedBooksWindow::PublishedBooksWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublishedBooksWindow)
    , m_networkManager(networkManager)
    , m_publisherId(0)
{
    ui->setupUi(this);

    // تنظیم سایز آیکون‌ها در QListWidget جهت نمایش صحیح کاور کتاب‌ها
    ui->publishedListWidget->setIconSize(QSize(120, 180));

    // اتصال سیگنال دریافت پاسخ شبکه به اسلات مربوطه
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &PublishedBooksWindow::handleResponse);

}

void PublishedBooksWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    fetchPublisherBooks();
}

PublishedBooksWindow::~PublishedBooksWindow()
{
    delete ui;
}

void PublishedBooksWindow::fetchPublisherBooks()
{
    m_publisherId = SessionManager::instance()->getUserId();
    if (m_publisherId <= 0) {
        qDebug() << "⚠️ invalid publisherId:" << m_publisherId;
        return;
    }

    QVariantMap params;
    params["publisherId"] = m_publisherId;

    Request request(CommandType::GetPublisherBooks, params);
    m_networkManager->sendRequest(request);
}

void PublishedBooksWindow::handleResponse(const Response& response)
{
    // ۱. پردازش لیست کتاب‌های دریافت‌شده از کامند GetPublisherBooks
    if (response.getCommandType() == CommandType::GetPublisherBooks) {

        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "مشکلی در دریافت کتاب‌ها به وجود آمد: " + response.getMessage());
            return;
        }

        QVariantList books = response.getData()["books"].toList();
        int count = response.getData()["count"].toInt();

        ui->publishedListWidget->clear();
        m_booksCache.clear();

        if (books.isEmpty()) {
            QMessageBox::information(this, "نتیجه", "هیچ کتابی توسط این ناشر منتشر نشده است.");
            return;
        }

        for (const QVariant& bookVar : books) {
            QVariantMap book = bookVar.toMap();
            int bookId = book["bookId"].toInt();
            QString title = book["title"].toString();
            QString author = book["author"].toString();
            QString coverPath = book["coverPath"].toString();

            // ذخیره در کش برای استفاده هنگام کلیک روی آیتم
            m_booksCache[bookId] = book;

            // ساخت آیتم گرافیکی در لیست
            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, bookId);

            item->setText("⏳ Loading...\n" + title + "\nنویسنده: " + author);
            item->setTextAlignment(Qt::AlignCenter);

            ui->publishedListWidget->addItem(item);

            // ارسال درخواست دریافت کاور کتاب در صورت وجود
            if (!coverPath.isEmpty() && bookId > 0) {
                m_networkManager->requestBookCover(bookId);
            } else {
                item->setText(title + "\nنویسنده: " + author);
            }
        }

        qDebug() << "✅ Successfully loaded" << count << "published books.";
    }
    // ۲. پردازش دریافت تصویر کاور کتاب‌ها
    else if (response.getCommandType() == CommandType::GetBookCover) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            int responseBookId = resData["bookId"].toInt();
            QString base64Data = resData["coverData"].toString();
            QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                QPixmap scaled = pixmap.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // جایگذاری کاور در آیتم متناظر
                for (int i = 0; i < ui->publishedListWidget->count(); ++i) {
                    QListWidgetItem* item = ui->publishedListWidget->item(i);
                    if (item && item->data(Qt::UserRole).toInt() == responseBookId) {
                        item->setIcon(QIcon(scaled));

                        if (m_booksCache.contains(responseBookId)) {
                            QVariantMap b = m_booksCache[responseBookId];
                            item->setText(b["title"].toString() + "\nنویسنده: " + b["author"].toString());
                        }
                        break;
                    }
                }
            }
        }
    }
}

void PublishedBooksWindow::on_publishedListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (m_booksCache.contains(id)) {
        // باز کردن پنجره جزییات کتاب مانند SearchWindow
        BookDetailDialog dialog(m_networkManager, m_booksCache[id], this);
        dialog.exec();
    }
}

void PublishedBooksWindow::on_backPushButton_clicked()
{
    emit backRequested();
}