#include "deleteBookWindow.h"
#include "ui_deleteBookWindow.h"
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "../appWindow/SessionManager.h"






class BookActionDialog : public QDialog {
public:
    enum Action { Delete, Deactivate, Cancel };

    BookActionDialog(const QString& bookTitle, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Manage Book");
        setMinimumWidth(320);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(15);

        QLabel* label = new QLabel(QString("What action do you want to perform on:\n\"%1\"?").arg(bookTitle), this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size: 14px; font-weight: bold;");
        layout->addWidget(label);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        QPushButton* deactivateBtn = new QPushButton("Deactivate", this);
        QPushButton* deleteBtn = new QPushButton("Delete", this);
        QPushButton* cancelBtn = new QPushButton("Cancel", this);

        // استایل‌دهی دکمه‌ها برای جذابیت بصری بیشتر
        deactivateBtn->setStyleSheet("background-color: #f0ad4e; color: white; padding: 8px; font-weight: bold; border-radius: 4px;");
        deleteBtn->setStyleSheet("background-color: #d9534f; color: white; padding: 8px; font-weight: bold; border-radius: 4px;");
        cancelBtn->setStyleSheet("padding: 8px; border-radius: 4px;");

        btnLayout->addWidget(deactivateBtn);
        btnLayout->addWidget(deleteBtn);
        btnLayout->addWidget(cancelBtn);
        layout->addLayout(btnLayout);

        // متصل کردن کلیک دکمه‌ها به مقدار خروجی دیالوگ
        connect(deactivateBtn, &QPushButton::clicked, this, [this](){ m_action = Deactivate; accept(); });
        connect(deleteBtn, &QPushButton::clicked, this, [this](){ m_action = Delete; accept(); });
        connect(cancelBtn, &QPushButton::clicked, this, [this](){ m_action = Cancel; reject(); });

        m_action = Cancel;
    }

    Action getSelectedAction() const { return m_action; }

private:
    Action m_action;
};

deleteBookWindow::deleteBookWindow(NetworkManager* networkManager,QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::deleteBookWindow)
    ,m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &deleteBookWindow::handleResponse);
}

deleteBookWindow::~deleteBookWindow()
{
    delete ui;
}
void deleteBookWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    requestPublisherBooks();
}

void deleteBookWindow::requestPublisherBooks()
{
    ui->booksListWidget->clear();

    QVariantMap params;
    params["publisherId"] = SessionManager::instance()->getUserId(); // دریافت آیدی ناشر لاگین شده

    Request request(CommandType::GetPublisherBooks, params);
    m_networkManager->sendRequest(request);
}


void deleteBookWindow::on_booksListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    // استخراج دیتای ذخیره شده در آیتم
    int bookId = item->data(Qt::UserRole).toInt();
    QString bookTitle = item->data(Qt::UserRole + 1).toString();

    // باز کردن دیالوگ تصمیم‌گیری
    BookActionDialog dialog(bookTitle, this);
    if (dialog.exec() == QDialog::Accepted) {
        BookActionDialog::Action action = dialog.getSelectedAction();

        QVariantMap params;
        params["bookId"] = bookId;

        if (action == BookActionDialog::Deactivate) {
            Request request(CommandType::DeactivateBook, params);
            m_networkManager->sendRequest(request);
        }
        else if (action == BookActionDialog::Delete) {
            params["reason"] = "Deleted by publisher"; // دلیل حذف کتاب
            Request request(CommandType::DeleteBook, params);
            m_networkManager->sendRequest(request);
        }
    }
}


void deleteBookWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // الف) پاسخ دریافت لیست کتاب‌ها
    if (type == CommandType::GetPublisherBooks) {
        if (response.isSuccess()) {
            QVariantMap data = response.getData();
            QVariantList booksList = data["books"].toList();

            ui->booksListWidget->clear();



            for (const QVariant& var : booksList) {
                QVariantMap book = var.toMap();

                QListWidgetItem* item = new QListWidgetItem(ui->booksListWidget);
                item->setSizeHint(QSize(0, 110)); // مشخص کردن ارتفاع هر ردیف در لیست

                // ساخت ویجت سفارشی حاوی عکس و اطلاعات متنی به صورت افقی (مثل بخش سرچ)
                QWidget* widget = new QWidget();
                QHBoxLayout* layout = new QHBoxLayout(widget);
                layout->setContentsMargins(10, 5, 10, 5);

                // بخش اول: عکس کاور کتاب
                QLabel* coverLabel = new QLabel();
                coverLabel->setFixedSize(70, 100);
                coverLabel->setScaledContents(true);
                coverLabel->setStyleSheet("border: 1px solid #ccc; border-radius: 4px;");

                QString coverPath = book["coverPath"].toString();
                if (!coverPath.isEmpty()) {
                    QPixmap pix(coverPath);
                    if (!pix.isNull()) {
                        coverLabel->setPixmap(pix.scaled(70, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    } else {
                        coverLabel->setText("No Cover");
                        coverLabel->setAlignment(Qt::AlignCenter);
                    }
                } else {
                    coverLabel->setText("No Cover");
                    coverLabel->setAlignment(Qt::AlignCenter);
                }

                // بخش دوم: جزییات کتاب
                QVBoxLayout* infoLayout = new QVBoxLayout();
                infoLayout->setSpacing(4);

                QLabel* titleLabel = new QLabel(book["title"].toString());
                titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

                QLabel* authorLabel = new QLabel("Author: " + book["author"].toString());
                authorLabel->setStyleSheet("color: #555; font-size: 12px;");

                QLabel* priceLabel = new QLabel(QString("Price: $%1").arg(book["price"].toDouble()));
                priceLabel->setStyleSheet("color: green; font-size: 12px; font-weight: bold;");

                infoLayout->addWidget(titleLabel);
                infoLayout->addWidget(authorLabel);
                infoLayout->addWidget(priceLabel);
                infoLayout->addStretch();

                layout->addWidget(coverLabel);
                layout->addLayout(infoLayout);
                layout->addStretch();

                // ذخیره کردن آیدی و عنوان کتاب در متادیتای آیتم لیست برای دسترسی‌های بعدی
                item->setData(Qt::UserRole, book["bookId"].toInt());
                item->setData(Qt::UserRole + 1, book["title"].toString());

                ui->booksListWidget->addItem(item);
                ui->booksListWidget->setItemWidget(item, widget); // الصاق ویجت شخصی‌سازی شده به آیتم لیست
            }
        } else {
            QMessageBox::critical(this, "Error", "Failed to load books: " + response.getMessage());
        }
    }
    // ب) پاسخ عملیات غیرفعال‌سازی کتاب
    else if (type == CommandType::DeactivateBook) {
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Book deactivated successfully!");
            requestPublisherBooks(); // رفرش کردن لیست کتاب‌ها
        } else {
            QMessageBox::critical(this, "Error", "Failed to deactivate: " + response.getMessage());
        }
    }
    // ج) پاسخ عملیات حذف کتاب
    else if (type == CommandType::DeleteBook) {
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success", "Book deleted successfully!");
            requestPublisherBooks(); // رفرش کردن لیست کتاب‌ها
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete: " + response.getMessage());
        }
    }
}
