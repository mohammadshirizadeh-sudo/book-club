#include "searchwindow.h"
#include "Users/ui_searchwindow.h"
#include "../Server/Request.h"
#include "BookDetailDialog.h"
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


void SearchWindow::on_showSearchPushButton_clicked()
{


    if(ui->searchBookPushButton->isChecked()){

        QString keyword = ui->searchLineEdit->text().trimmed();
        if (keyword.isEmpty()) {
            QMessageBox::warning(this, "خطا", "لطفاً کلمه کلیدی یا عنوان کتاب را برای جستجو وارد کنید.", QMessageBox::Ok);
            return;
        }


        QVariantMap params;
        params["keyword"] = keyword;

        Request request(CommandType::SearchBooks, params);

        qDebug() << "🔍 [Client] Sending SearchBooks request with keyword:" << keyword;

        m_networkManager->sendRequest(request);

    }

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



void SearchWindow::on_searchResultsListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    // ۱. استخراج آیدی کتاب که قبلاً در این آیتم ذخیره کرده بودیم
    int bookId = item->data(Qt::UserRole).toInt();

    // ۲. پیدا کردن اطلاعات کامل آن کتاب از روی "کش" پنجره جستجو
    if (m_searchBooksCache.contains(bookId)) {
        QVariantMap selectedBookData = m_searchBooksCache[bookId];

        qDebug() << "📖 Opening details for book:" << selectedBookData["title"].toString();

        // ۳. ساخت دیالوگ به صورت مودال و پاس دادن کل اطلاعات کتاب به آن
        BookDetailDialog dialog(selectedBookData, this);

        // ۴. نمایش دیالوگ روی صفحه قبلی
        dialog.exec();
    }
}

