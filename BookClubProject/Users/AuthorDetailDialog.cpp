#include "AuthorDetailDialog.h"
#include "Users/ui_AuthorDetailDialog.h" // مسیر تولید شده توسط uic برای دیالوگ شما
#include "BookDetailDialog.h"
#include <QPixmap>
#include <QDebug>
#include "../Server/Request.h"
#include "../Server/Response.h"

AuthorDetailDialog::AuthorDetailDialog(const QVariantMap& authorData,NetworkManager* networkManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorDetailDialog),
    m_networkManager(networkManager)
{
    ui->setupUi(this);
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &AuthorDetailDialog::onResponseReceived);
    displayAuthorInfo(authorData);
}

AuthorDetailDialog::~AuthorDetailDialog()
{
    delete ui;
}

void AuthorDetailDialog::displayAuthorInfo(const QVariantMap& authorData)
{
    // ۱. تنظیم نام نویسنده و تعداد کتاب‌ها در لیبل‌های مربوطه
    ui->authorNameLabel->setText(authorData["author"].toString());
    ui->booksCountLabel->setText(authorData["bookCount"].toString() + " جلد کتاب");

    // ۲. پاکسازی لیست و کش محلی پیش از بارگذاری داده‌های جدید
    ui->booksListWidget->clear();
    m_authorBooksCache.clear();

    // ۳. استخراج و نمایش لیست کتاب‌های این نویسنده
    QVariantList books = authorData["books"].toList();
    for (const QVariant& bookVar : books) {
        QVariantMap book = bookVar.toMap();
        int bookId = book["bookId"].toInt();
        QString title = book["title"].toString();
        QString author = book["author"].toString();
        QString coverPath = book["coverPath"].toString();

        // ذخیره اطلاعات کامل کتاب در کش دیالوگ نویسنده
        m_authorBooksCache[bookId] = book;

        // ساخت آیتم جدید در QListWidget
        QListWidgetItem* item = new QListWidgetItem();
        item->setData(Qt::UserRole, bookId); // ذخیره کردن آیدی کتاب در دیتای آیتم
        item->setText(title + "\n" + author);
        item->setTextAlignment(Qt::AlignCenter);

        ui->booksListWidget->addItem(item);
        if (!coverPath.isEmpty() && bookId > 0) {
            m_networkManager->requestBookCover(bookId);
        }
    }
}


void AuthorDetailDialog::onResponseReceived(const Response& response)
{
    if (response.getCommandType() == CommandType::GetBookCover) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData(); // اصلاح شده بر اساس پچ قبلی (بدون .toMap)
            int responseBookId = resData["bookId"].toInt();
            QString base64Data = resData["coverData"].toString();
            QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                QPixmap scaled = pixmap.scaled(80, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // 🟢 پیدا کردن آیتم متناظر با این عکس در QListWidget و اعمال آیکون روی آن
                for (int i = 0; i < ui->booksListWidget->count(); ++i) {
                    QListWidgetItem* item = ui->booksListWidget->item(i);
                    if (item && item->data(Qt::UserRole).toInt() == responseBookId) {
                        item->setIcon(QIcon(scaled));

                        // بازگرداندن متن به حالت اولیه و حذف کلمه Loading
                        if (m_authorBooksCache.contains(responseBookId)) {
                            QVariantMap b = m_authorBooksCache[responseBookId];
                            item->setText(b["title"].toString() + "\n" + b["author"].toString());
                        }
                        break;
                    }
                }
            }
        }
    }
}

void AuthorDetailDialog::on_booksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    // استخراج آیدی کتاب کلیک شده
    int bookId = item->data(Qt::UserRole).toInt();

    if (m_authorBooksCache.contains(bookId)) {
        QVariantMap selectedBookData = m_authorBooksCache[bookId];

        qDebug() << "📖 Opening BookDetailDialog from author profile for:" << selectedBookData["title"].toString();

        // باز کردن دیالوگ جزئیات کتاب به صورت مودال (Modal) روی همین صفحه اطلاعات نویسنده
        BookDetailDialog dialog( m_networkManager ,selectedBookData, this);
        dialog.exec();
    }
}