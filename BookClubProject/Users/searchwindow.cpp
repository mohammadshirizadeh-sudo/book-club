#include "searchwindow.h"
#include "Users/ui_searchwindow.h"
#include "../Server/Request.h"
#include "../Server/Response.h" // 🟢 اضافه شده برای پردازش ساختار پاسخ سرور
#include "BookDetailDialog.h"
#include "UserDetailDialog.h"
#include "AuthorDetailDialog.h"
#include <QListWidgetItem>

#include <QMessageBox>
#include <QDebug>
#include <QGroupBox>
#include <QButtonGroup>

SearchWindow::SearchWindow(NetworkManager* networkManager , QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    ui->searchBookPushButton->setCheckable(true);
    ui->searchAuthorPushButton->setCheckable(true);
    ui->searchPublisherPushButton->setCheckable(true);

    // ۲. ایجاد یک گروه برای دکمه‌ها تا رفتار انحصاری (Exclusive) داشته باشند
    QButtonGroup* searchTypeGroup = new QButtonGroup(this);
    searchTypeGroup->addButton(ui->searchBookPushButton);
    searchTypeGroup->addButton(ui->searchAuthorPushButton);
    searchTypeGroup->addButton(ui->searchPublisherPushButton);

    // انحصاری کردن گروه: با فشرده شدن یکی، بقیه خودکار بالا می‌آیند
    searchTypeGroup->setExclusive(true);

    // ۳. به صورت پیش‌فرض دکمه کتاب را فعال می‌گذاریم
    ui->searchBookPushButton->setChecked(true);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &SearchWindow::handleResponse);
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

void SearchWindow:: on_showSearchPushButton_clicked()
{
    QString keyword = ui->searchLineEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً کلمه کلیدی را وارد کنید.");
        return;
    }

    QVariantMap params;
    params["keyword"] = keyword;
    if (ui->searchBookPushButton->isChecked()){
        params["status"] = "book";
    }else if(ui->searchPublisherPushButton->isChecked()){
        qDebug()<<"we are in onshow clicked";
        params["status"] = "publisher";
    }else if(ui->searchAuthorPushButton->isChecked()){
        params["status"] = "author";
    }

    Request request(CommandType::SearchBooks, params);
    m_networkManager->sendRequest(request);
}

void SearchWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::SearchBooks) {

        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "مشکلی در جستجو به وجود آمد: " + response.getMessage());
            return;
        }

        QVariantList books = response.getData()["books"].toList();
        int count = response.getData()["count"].toInt();

        ui->searchResultsListWidget->clear();
        m_searchBooksCache.clear();

        if (books.isEmpty()) {
            QMessageBox::information(this, "نتیجه", "هیچ کتابی با این مشخصات پیدا نشد.");
            return;
        }

        // ۳. پیمایش لیست کتاب‌های دریافتی و اضافه کردن به QListWidget
        for (const QVariant& bookVar : books) {
            QVariantMap book = bookVar.toMap();
            int bookId = book["bookId"].toInt();
            QString title = book["title"].toString();
            QString author = book["author"].toString();
            QString coverPath = book["coverPath"].toString();

            // ذخیره در Cache پنجره جستجو برای استفاده در زمان کلیک
            m_searchBooksCache[bookId] = book;

            // ایجاد آیتم جدید در لیست
            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, bookId);

            // 🟢 تغییر اول: تنظیم متن موقت در وضعیت Loading تا زمان لود آنلاین دیتای عکس از سرور
            item->setText("⏳ Loading...\n" + title + "\n" + author);
            item->setTextAlignment(Qt::AlignCenter);

            ui->searchResultsListWidget->addItem(item);
            if (!coverPath.isEmpty() && bookId > 0) {
                m_networkManager->requestBookCover(bookId);
            } else {
                // اگر کتاب فاقد عکس بود، متن عادی بدون پیشوند لودینگ ست می‌شود
                item->setText(title + "\n" + author);
            }
        }

        qDebug() << "✅ Successfully loaded" << count << "search results.";
    }
    // 🟢 تغییر سوم: اضافه کردن پردازش پاسخ کامند GetBookCover جهت دریافت تصاویر جلدها به صورت ناهمگام
    else if (response.getCommandType() == CommandType::GetBookCover) {
        if (response.isSuccess()) {
            QVariantMap resData = response.getData();
            int responseBookId = resData["bookId"].toInt();
            QString base64Data = resData["coverData"].toString();
            QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());

            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                QPixmap scaled = pixmap.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // پیدا کردن آیتم متناظر با عکس دریافتی در لیست گرافیکی و اعمال تغییرات روی آن
                for (int i = 0; i < ui->searchResultsListWidget->count(); ++i) {
                    QListWidgetItem* item = ui->searchResultsListWidget->item(i);
                    if (item && item->data(Qt::UserRole).toInt() == responseBookId) {
                        item->setIcon(QIcon(scaled));

                        // بازگرداندن متن آیتم به حالت اصلی و حذف کلمه Loading
                        if (m_searchBooksCache.contains(responseBookId)) {
                            QVariantMap b = m_searchBooksCache[responseBookId];
                            item->setText(b["title"].toString() + "\n" + b["author"].toString());
                        }
                        break;
                    }
                }
            }
        }
    }
}

void SearchWindow::on_searchResultsListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (m_searchBooksCache.contains(id)) {
        // 🟢 تغییر چهارم: اصلاح ترتیب آرگومان‌ها مطابق با پیاده‌سازی گام قبلی کلاس BookDetailDialog
        // ابتدا دیتای کتاب (آرگومان اول)، سپس پوینتر شبکه (آرگومان دوم)
        BookDetailDialog dialog(m_networkManager ,m_searchBooksCache[id], this);
        dialog.exec();
    }
}