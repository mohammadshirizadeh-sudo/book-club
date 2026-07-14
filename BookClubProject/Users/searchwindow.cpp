#include "searchwindow.h"
#include "Users/ui_searchwindow.h"
#include "../Server/Request.h"
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

    if (ui->searchBookPushButton->isChecked()) {
        Request request(CommandType::SearchBooks, params);
        m_networkManager->sendRequest(request);
    }

    else if (ui->searchPublisherPushButton->isChecked()) {
        Request request(CommandType::SearchUsers, params);
        m_networkManager->sendRequest(request);
    }else if (ui->searchAuthorPushButton->isChecked()) {
        Request request(CommandType::SearchAuthors, params);
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



    if (response.getCommandType() == CommandType::SearchUsers) {
        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "خطا در جستجو: " + response.getMessage());
            return;
        }

        QVariantList users = response.getData()["users"].toList();
        int count = response.getData()["count"].toInt();

        ui->searchResultsListWidget->clear();
        m_searchUsersCache.clear();

        if (users.isEmpty()) {
            QMessageBox::information(this, "نتیجه", "هیچ کاربری پیدا نشد.");
            return;
        }

        for (const QVariant& userVar : users) {
            QVariantMap user = userVar.toMap();
            int userId = user["id"].toInt();
            QString role = user["role"].toString();
            if(!user.contains("publisherName")) continue;
            // if(role != "publisher") continue;

            // اگر ناشر بود نام انتشارات وگرنه نام کاربری را نشان می‌دهیم
            QString displayName = user.contains("publisherName") ? user["publisherName"].toString() : user["username"].toString();

            // ذخیره در کش کاربران
            m_searchUsersCache[userId] = user;

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, userId);
            item->setText(displayName + "\n" + "نقش: " + role);
            item->setTextAlignment(Qt::AlignCenter);

            ui->searchResultsListWidget->addItem(item);
        }
        qDebug() << "✅ Loaded" << count << "users/publishers.";
    }


    if (response.getCommandType() == CommandType::SearchAuthors) {
        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "مشکلی در جستجو به وجود آمد: " + response.getMessage());
            return;
        }

        QVariantList authors = response.getData()["authors"].toList();
        int count = response.getData()["count"].toInt();

        ui->searchResultsListWidget->clear();
        m_searchAuthorsCache.clear();

        if (authors.isEmpty()) {
            QMessageBox::information(this, "نتیجه", "هیچ نویسنده‌ای با این مشخصات پیدا نشد.");
            return;
        }

        int fakeId = 0; // استفاده از اندکس به عنوان آیدی یکتا در کش محلی
        for (const QVariant& authorVar : authors) {
            QVariantMap authorData = authorVar.toMap();
            QString authorName = authorData["author"].toString();
            int bookCount = authorData["bookCount"].toInt();

            // ذخیره در کش نویسندگان
            m_searchAuthorsCache[fakeId] = authorData;

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, fakeId);
            item->setText(authorName + "\n" + QString::number(bookCount) + " کتاب");
            item->setTextAlignment(Qt::AlignCenter);

            // 🎨 شیک‌سازی: لود کاور اولین کتاب به عنوان تصویر آیکون نویسنده
            QVariantList books = authorData["books"].toList();
            if (!books.isEmpty()) {
                QVariantMap firstBook = books.first().toMap();
                QString coverPath = firstBook["coverPath"].toString();
                if (!coverPath.isEmpty()) {
                    QPixmap pixmap(coverPath);
                    if (!pixmap.isNull()) {
                        QPixmap scaled = pixmap.scaled(120, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        item->setIcon(QIcon(scaled));
                    }
                }
            }
            ui->searchResultsListWidget->addItem(item);
            fakeId++;
        }
        qDebug() << "✅ Successfully loaded" << count << "authors.";
    }
}



void SearchWindow::on_searchResultsListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (ui->searchBookPushButton->isChecked() && m_searchBooksCache.contains(id)) {
        BookDetailDialog dialog(m_searchBooksCache[id], this);
        dialog.exec();
    }
    else if ((ui->searchPublisherPushButton->isChecked())
             && m_searchUsersCache.contains(id)) {

        UserDetailDialog dialog(m_searchUsersCache[id], this);
        dialog.exec();
    }else if (ui->searchAuthorPushButton->isChecked() && m_searchAuthorsCache.contains(id)) {
        AuthorDetailDialog dialog(m_searchAuthorsCache[id], this);
        dialog.exec();
    }
}
