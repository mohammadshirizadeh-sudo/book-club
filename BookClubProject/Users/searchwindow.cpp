#include "searchwindow.h"
#include "Users/ui_searchwindow.h"
#include "../Server/Request.h"
#include <QListWidgetItem>

#include <QMessageBox>
#include <QDebug>

SearchWindow::SearchWindow(NetworkManager* networkManager , QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &SearchWindow::handleResponse);
}

SearchWindow::~SearchWindow()
{
    delete ui;
}


void SearchWindow::on_searchBookPushButton_clicked()
{
    // ۱. دریافت عبارت جستجو و حذف فاصله‌های اضافی ابتدای و انتها
    QString keyword = ui->searchLineEdit->text().trimmed();

    // ۲. اعتبارسنحی اولیه (اگر کاربر چیزی تایپ نکرده باشد، هشدار می‌دهیم)
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً کلمه کلیدی یا عنوان کتاب را برای جستجو وارد کنید.", QMessageBox::Ok);
        return;
    }

    // ۳. بسته‌بندی پارامترها با کلید دقیقاً یکسان با سرور ("keyword")
    QVariantMap params;
    params["keyword"] = keyword;

    // ۴. ساخت شیء درخواست با کامند مربوطه
    Request request(CommandType::SearchBooks, params);

    qDebug() << "🔍 [Client] Sending SearchBooks request with keyword:" << keyword;

    // ۵. ارسال درخواست به سرور
    m_networkManager->sendRequest(request);
}

void SearchWindow::handleResponse(const Response& response)
{
    // مدیریت پاسخ درخواست جستجوی کتاب
    if (response.getCommandType() == CommandType::SearchBooks) {

        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "مشکلی در جستجو به وجود آمد: " + response.getMessage());
            return;
        }

        // ۱. استخراج داده‌ها از پاسخ سرور
        QVariantList books = response.getData()["books"].toList();
        int count = response.getData()["count"].toInt();

        // ۲. پاکسازی لیست قبلی و کش مربوط به جستجو
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

            // تنظیم متن (عنوان + نویسنده)
            item->setText(title + "\n" + author);
            item->setTextAlignment(Qt::AlignCenter);

            // تنظیم تصویر جلد کتاب (در صورت وجود)
            if (!coverPath.isEmpty()) {
                QPixmap pixmap(coverPath);
                if (!pixmap.isNull()) {
                    QPixmap scaled = pixmap.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    item->setIcon(QIcon(scaled));
                }
            }

            ui->searchResultsListWidget->addItem(item);
        }

        qDebug() << "✅ Successfully loaded" << count << "search results.";
    }
}