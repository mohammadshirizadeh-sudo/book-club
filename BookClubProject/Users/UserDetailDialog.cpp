#include "UserDetailDialog.h"
#include "Users/ui_UserDetailDialog.h"
#include "BookDetailDialog.h"


UserDetailDialog::UserDetailDialog(const QVariantMap& userData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserDetailDialog) // یا نامی که uic تولید کرده است
{
    ui->setupUi(this);
    displayUserInfo(userData);
}

UserDetailDialog::~UserDetailDialog()
{
    delete ui;
}

void UserDetailDialog::displayUserInfo(const QVariantMap& userData)
{
    ui->usernameLabel->setText(userData["username"].toString());
    ui->emailLabel->setText(userData["email"].toString());
    ui->roleLabel->setText(userData["role"].toString());

    ui->booksListWidget->clear();
    m_publisherBooksCache.clear();
    if (userData.contains("publisherName")) {
        ui->label_4->setText(userData["publisherName"].toString());
        ui->revenueLabel->setText(userData["totalRevenue"].toString() + " تومان");
        ui->joinedLabel->setText(userData["joinedAt"].toString());
        ui->booksCountLabel->setText(userData["publishedBooksCount"].toString() + " جلد");

        // استخراج و نمایش لیست کتاب‌های این ناشر
        QVariantList books = userData["books"].toList();
        ui->booksListWidget->clear();
        for (const QVariant& bookVar : books) {
            QVariantMap book = bookVar.toMap();
            int bookId = book["bookId"].toInt();
            QString title = book["title"].toString();
            QString author = book["author"].toString();
            QString coverPath = book["coverPath"].toString();
            m_publisherBooksCache[bookId] = book;

            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, bookId);
            item->setText(title + "\n" + author);
            item->setTextAlignment(Qt::AlignCenter);
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
}



void UserDetailDialog::on_booksListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    int bookId = item->data(Qt::UserRole).toInt();

    if (m_publisherBooksCache.contains(bookId)) {
        QVariantMap selectedBookData = m_publisherBooksCache[bookId];
        qDebug() << "📖 Opening BookDetailDialog from publisher profile for:" << selectedBookData["title"].toString();
        BookDetailDialog dialog(selectedBookData, this);
        dialog.exec();
    }
}