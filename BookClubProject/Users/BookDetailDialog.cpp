
#include"BookDetailDialog.h"
#include "Users/ui_BookDetailDialog.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include <QPixmap>

BookDetailDialog::BookDetailDialog(NetworkManager*networkManager , const QVariantMap& bookData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookDetailDialog),
    m_networkManager(networkManager),
    m_bookData(bookData)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &BookDetailDialog::onResponseReceived);


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
    ui->priceLabel->setText(bookData["price"].toString() + " Tooman");
    ui->discountLabel->setText(bookData["discountPercent"].toString() + " ٪");
    ui->finalPriceLabel->setText(bookData["finalPrice"].toString() + " Tooman");
    ui->ratingLabel->setText(bookData["averageRating"].toString());

    int bookId = bookData["bookId"].toInt();
    QString coverPath = bookData["coverPath"].toString();
    if (!coverPath.isEmpty() && bookId > 0) {
        ui->coverLable->setAlignment(Qt::AlignCenter);
        ui->coverLable->setText("Loading Cover..."); // نمایش وضعیت بارگذاری به کاربر

        // 🟢 ارسال درخواست دریافت عکس به سرور (دقیقاً مشابه متد درون UserWindow)
        m_networkManager->requestBookCover(bookId);
    } else {
        ui->coverLable->setText("No Cover Image");
    }
}




void BookDetailDialog::onResponseReceived(const Response& response)
{
    // 🔴 توجه: نام Enum کامند دریافت عکس را بر اساس پروژه خود تنظیم کنید (مثلاً GetBookCover)
    if (response.getCommandType() == CommandType::GetBookCover) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            int responseBookId = resData["bookId"].toInt();
            int currentBookId = m_bookData["bookId"].toInt();

            // بررسی اینکه عکس دریافت شده دقیقاً متعلق به همین کتابِ باز شده باشد
            if (responseBookId == currentBookId) {
                // استخراج رشته متنی Base64 و تبدیل مجدد آن به دیتای باینری
                QString base64Data = resData["coverData"].toString();
                QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

                QPixmap pixmap;
                // لود کردن عکس از دیتای باینری دریافتی
                if (pixmap.loadFromData(imageData)) {
                    // اسکیل کردن عکس دقیقاً به اندازه خودِ لیبل با کیفیت بالا
                    QPixmap scaledPixmap = pixmap.scaled(ui->coverLable->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation);
                    ui->coverLable->setPixmap(scaledPixmap);
                    ui->coverLable->setText(""); // حذف متن Loading پس از موفقیت
                } else {
                    ui->coverLable->setText("Failed to process image data");
                }
            }
        } else {
            ui->coverLable->setText("Failed to download cover");
        }
    }
}
void BookDetailDialog::on_addFavoritePushButton_clicked()
{

}

