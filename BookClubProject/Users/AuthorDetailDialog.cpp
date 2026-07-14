#include "AuthorDetailDialog.h"
#include "Users/ui_AuthorDetailDialog.h" // مسیر تولید شده توسط uic برای دیالوگ شما
#include "BookDetailDialog.h"
#include <QPixmap>
#include <QDebug>

AuthorDetailDialog::AuthorDetailDialog(const QVariantMap& authorData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorDetailDialog)
{
    ui->setupUi(this);
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

        // 🖼️ لود و تنظیم عکس جلد کتاب با ابعاد مناسب (۸۰ در ۱۲۰) مطابق با سلیقه شما
        if (!coverPath.isEmpty()) {
            QPixmap pixmap(coverPath);
            if (!pixmap.isNull()) {
                QPixmap scaled = pixmap.scaled(80, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                item->setIcon(QIcon(scaled));
            }
        }
        ui->booksListWidget->addItem(item);
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
        BookDetailDialog dialog(selectedBookData, this);
        dialog.exec();
    }
}