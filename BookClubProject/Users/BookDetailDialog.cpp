
#include"BookDetailDialog.h"
#include "Users/ui_BookDetailDialog.h"
#include <QPixmap>

BookDetailDialog::BookDetailDialog(const QVariantMap& bookData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookDetailDialog)
{
    ui->setupUi(this);

    // نمایش اطلاعات کتاب به محض باز شدن دیالوگ
    displayBookInfo(bookData);
}

BookDetailDialog::~BookDetailDialog()
{
    delete ui;
}

void BookDetailDialog::displayBookInfo(const QVariantMap& bookData)
{
    // پر کردن لِیبل‌ها بر اساس کلیدهایی که سرور پاس داده است
    ui->titleLabel->setText(bookData["title"].toString());
    ui->authorLabel->setText(bookData["author"].toString());
    ui->genreLabel->setText(bookData["genre"].toString());
    ui->priceLabel->setText(bookData["price"].toString() + " تومان");
    ui->discountLabel->setText(bookData["discountPercent"].toString() + " ٪");
    ui->finalPriceLabel->setText(bookData["finalPrice"].toString() + " تومان");
    ui->ratingLabel->setText(bookData["averageRating"].toString());

    // نمایش تصویر جلد کتاب در ابعاد بزرگتر در دیالوگ
    QString coverPath = bookData["coverPath"].toString();
    if (!coverPath.isEmpty()) {
        QPixmap pixmap(coverPath);
        if (!pixmap.isNull()) {
            ui->coverLable->setPixmap(pixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}