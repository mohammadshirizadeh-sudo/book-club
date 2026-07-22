#include "deactivatebookwindow.h"
#include "Publishers/ui_deactivatebookwindow.h"
#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QPixmap>

DeactivateBookWindow::DeactivateBookWindow(NetworkManager* networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeactivateBookWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    // اتصال سیگنال شبکه
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &DeactivateBookWindow::handleResponse);

    // تنظیمات اولیه جدول
    ui->booksTable->setColumnWidth(0, 60);   // ID
    ui->booksTable->setColumnWidth(1, 80);   // Cover
    ui->booksTable->setColumnWidth(2, 250);  // Title
    ui->booksTable->setColumnWidth(3, 180);  // Author
    ui->booksTable->setColumnWidth(4, 100);  // Price
    ui->booksTable->setColumnWidth(5, 120);  // Status
    ui->booksTable->setColumnWidth(6, 90);   // Sales
}

DeactivateBookWindow::~DeactivateBookWindow()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &DeactivateBookWindow::handleResponse);
    delete ui;
}

void DeactivateBookWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadPublisherBooks();
}

// 🟢 درخواست بارگذاری کتاب‌های ناشر
void DeactivateBookWindow::loadPublisherBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    if (publisherId <= 0) return;

    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetPublisherBooks, params);
    m_networkManager->sendRequest(request);
}

// 🟢 مدیریت پاسخ‌های دریافتی از سرور
void DeactivateBookWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    if (type != CommandType::GetPublisherBooks &&
        type != CommandType::DeactivateBook &&
        type != CommandType::ReactivateBook) {
        return;
    }

    if (!response.isSuccess()) {
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    // ۱. لیست کتاب‌ها دریافت شد
    if (type == CommandType::GetPublisherBooks) {
        m_allBooks.clear();
        QVariantList books = data["books"].toList();

        for (const QVariant& var : books) {
            m_allBooks.append(var.toMap());
        }

        populateTable();
    }
    // ۲. غیرفعال‌سازی یا فعال‌سازی مجدد موفقیت‌آمیز بود
    else if (type == CommandType::DeactivateBook || type == CommandType::ReactivateBook) {
        QMessageBox::information(this, "Success", response.getMessage());
        loadPublisherBooks(); // بروزرسانی جدول
    }
}

// 📋 پر کردن جدول بر اساس فیلترها و جستجو
void DeactivateBookWindow::populateTable()
{
    ui->booksTable->clearContents();
    ui->booksTable->setRowCount(0);

    QString filterText = ui->searchLineEdit->text().trimmed().toLower();
    bool showAll = ui->allBooksRadio->isChecked();
    bool showActiveOnly = ui->activeBooksRadio->isChecked();

    int row = 0;
    for (const auto& book : m_allBooks) {
        bool isActive = book["isActive"].toBool();
        QString title = book["title"].toString();
        QString author = book["author"].toString();

        // اعمال فیلتر وضعیت
        if (showActiveOnly && !isActive) continue;
        if (ui->deactivatedBooksRadio->isChecked() && isActive) continue;

        // اعمال فیلتر جستجو
        if (!filterText.isEmpty()) {
            if (!title.toLower().contains(filterText) && !author.toLower().contains(filterText)) {
                continue;
            }
        }

        ui->booksTable->insertRow(row);

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(book["bookId"].toInt()));
        idItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 0, idItem);

        // Cover Thumbnail
        QString coverPath = book["coverPath"].toString();
        QLabel* coverLabel = new QLabel();
        coverLabel->setAlignment(Qt::AlignCenter);
        if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
            QPixmap pix(coverPath);
            coverLabel->setPixmap(pix.scaled(40, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            coverLabel->setText("📖");
        }
        ui->booksTable->setCellWidget(row, 1, coverLabel);

        // Title & Author
        ui->booksTable->setItem(row, 2, new QTableWidgetItem(title));
        ui->booksTable->setItem(row, 3, new QTableWidgetItem(author));

        // Price
        double price = book["price"].toDouble();
        ui->booksTable->setItem(row, 4, new QTableWidgetItem(QString("$%1").arg(price, 0, 'f', 2)));

        // Status
        QTableWidgetItem* statusItem = new QTableWidgetItem(isActive ? "🟢 Active" : "🔴 Deactivated");
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 5, statusItem);

        // Sales
        int sales = book["salesCount"].toInt();
        QTableWidgetItem* salesItem = new QTableWidgetItem(QString::number(sales));
        salesItem->setTextAlignment(Qt::AlignCenter);
        ui->booksTable->setItem(row, 6, salesItem);

        // Publish Date
        QString publishDate = book["publishDate"].toString();
        ui->booksTable->setItem(row, 7, new QTableWidgetItem(publishDate.isEmpty() ? "N/A" : publishDate));

        // Actions column info text
        ui->booksTable->setItem(row, 8, new QTableWidgetItem(isActive ? "Available" : "Hidden"));

        row++;
    }

    updateSelectedBookUI();
}

// 📖 تغییر ردیف انتخابی در جدول
void DeactivateBookWindow::on_booksTable_itemSelectionChanged()
{
    int currentRow = ui->booksTable->currentRow();
    if (currentRow < 0) {
        m_selectedBookId = -1;
        m_selectedBookData.clear();
    } else {
        QTableWidgetItem* idItem = ui->booksTable->item(currentRow, 0);
        if (idItem) {
            m_selectedBookId = idItem->text().toInt();
            for (const auto& book : m_allBooks) {
                if (book["bookId"].toInt() == m_selectedBookId) {
                    m_selectedBookData = book;
                    break;
                }
            }
        }
    }
    updateSelectedBookUI();
}

// 🔄 بروزرسانی وضعیت دکمه‌ها و لیبل کتاب انتخابی
void DeactivateBookWindow::updateSelectedBookUI()
{
    if (m_selectedBookId <= 0 || m_selectedBookData.isEmpty()) {
        ui->selectedBookLabel->setText("Selected: No book selected");
        ui->deactivateBookButton->setEnabled(false);
        ui->reactivateBookButton->setEnabled(false);
        ui->viewDetailsButton->setEnabled(false);
        return;
    }

    QString title = m_selectedBookData["title"].toString();
    bool isActive = m_selectedBookData["isActive"].toBool();

    ui->selectedBookLabel->setText(QString("Selected: %1 (%2)").arg(title, isActive ? "Active" : "Deactivated"));

    // اگر کتاب فعال است، دکمه Deactivate فعال می‌شود
    ui->deactivateBookButton->setEnabled(isActive);
    // اگر غیرفعال است، دکمه Reactivate فعال می‌شود
    ui->reactivateBookButton->setEnabled(!isActive);

    ui->viewDetailsButton->setEnabled(true);
}

// 🚫 غیرفعال‌سازی کتاب
void DeactivateBookWindow::on_deactivateBookButton_clicked()
{
    if (m_selectedBookId <= 0) return;

    auto reply = QMessageBox::question(this, "Confirm Deactivation",
                                       QString("Are you sure you want to deactivate '%1'?\nIt will be hidden from the store.")
                                           .arg(m_selectedBookData["title"].toString()),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QVariantMap params;
        params["bookId"] = m_selectedBookId;

        Request request(CommandType::DeactivateBook, params);
        m_networkManager->sendRequest(request);
    }
}

// ✅ فعال‌سازی مجدد کتاب
void DeactivateBookWindow::on_reactivateBookButton_clicked()
{
    if (m_selectedBookId <= 0) return;

    QVariantMap params;
    params["bookId"] = m_selectedBookId;

    Request request(CommandType::ReactivateBook, params);
    m_networkManager->sendRequest(request);
}

// 👁️ مشاهده جزئیات کتاب
void DeactivateBookWindow::on_viewDetailsButton_clicked()
{
    if (m_selectedBookData.isEmpty()) return;

    QString details = QString("Title: %1\nAuthor: %2\nPrice: $%3\nSales: %4\nStatus: %5\n\nDescription:\n%6")
                          .arg(m_selectedBookData["title"].toString())
                          .arg(m_selectedBookData["author"].toString())
                          .arg(m_selectedBookData["price"].toDouble(), 0, 'f', 2)
                          .arg(m_selectedBookData["salesCount"].toInt())
                          .arg(m_selectedBookData["isActive"].toBool() ? "Active" : "Deactivated")
                          .arg(m_selectedBookData["description"].toString());

    QMessageBox::information(this, "Book Details", details);
}

// 🔍 فیلترها و جستجو
void DeactivateBookWindow::on_searchLineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);
    populateTable();
}

void DeactivateBookWindow::on_allBooksRadio_toggled(bool checked)
{
    if (checked) populateTable();
}

void DeactivateBookWindow::on_activeBooksRadio_toggled(bool checked)
{
    if (checked) populateTable();
}

void DeactivateBookWindow::on_deactivatedBooksRadio_toggled(bool checked)
{
    if (checked) populateTable();
}

void DeactivateBookWindow::on_refreshListButton_clicked()
{
    loadPublisherBooks();
}

// 📥 خروجی CSV
void DeactivateBookWindow::on_exportListButton_clicked()
{
    exportToCSV();
}

void DeactivateBookWindow::exportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Inventory to CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Cannot write to file.");
        return;
    }

    QTextStream out(&file);
    out << "ID,Title,Author,Price,Status,SalesCount\n";

    for (const auto& book : m_allBooks) {
        out << book["bookId"].toInt() << ","
            << "\"" << book["title"].toString() << "\","
            << "\"" << book["author"].toString() << "\","
            << book["price"].toDouble() << ","
            << (book["isActive"].toBool() ? "Active" : "Deactivated") << ","
            << book["salesCount"].toInt() << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Success", "Books inventory exported successfully.");
}

// ⬅ بازگشت
void DeactivateBookWindow::on_backPushButton_clicked()
{
    this->close();
}