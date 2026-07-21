#include "shelfmanagementdialog.h"
#include "Users/ui_shelfmanagementdialog.h"
#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>


ShelfManagementDialog::ShelfManagementDialog(NetworkManager* networkManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShelfManagementDialog),
    m_networkManager(networkManager),
    m_currentShelfId(-1)
{
    ui->setupUi(this);

    // اتصال سیگنال دریافت پاسخ از سرور
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ShelfManagementDialog::handleResponse);

    // پاکسازی فرم جزئیات کتاب در ابتدا
    clearBookDetails();
}

void ShelfManagementDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    loadShelves();
}
ShelfManagementDialog::~ShelfManagementDialog()
{
    // 🟢 قطع اتصال سیگنال برای جلوگیری از تداخل پیام‌های شبکه
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &ShelfManagementDialog::handleResponse);
    delete ui;
}

// 🟢 درخواست دریافت قفسه‌های کاربر
void ShelfManagementDialog::loadShelves()
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0) return;


    m_currentShelfId=-1;

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetUserShelves, params);
    m_networkManager->sendRequest(request);
}

// 🟢 درخواست دریافت کتاب‌های موجود در یک قفسه
void ShelfManagementDialog::loadBooksInShelf(int shelfId)
{
    int userId = SessionManager::instance()->getUserId();
    if (userId <= 0 || shelfId <= 0) return;

    QVariantMap params;
    params["userId"] = userId;
    params["shelfId"] = shelfId;

    Request request(CommandType::GetBooksInShelf, params);
    m_networkManager->sendRequest(request);
}

// 🟢 پردازش مرکزی پاسخ‌های سرور
void ShelfManagementDialog::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // فیلتر کردن دستورات غیرمرتبط
    if (type != CommandType::GetUserShelves &&
        type != CommandType::GetBooksInShelf &&
        type != CommandType::CreateShelf &&
        type != CommandType::DeleteShelf &&
        type != CommandType::RenameShelf &&
        type != CommandType::RemoveBookFromShelf &&
        type != CommandType::MoveBookBetweenShelves) {
        return;
    }

    if (!response.isSuccess()) {
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    // ۱. لیست قفسه‌ها دریافت شد
    if (type == CommandType::GetUserShelves) {
        ui->shelfList->clear();
        m_shelvesMap.clear();

        QVariantList shelves = data["shelves"].toList();
        for (const QVariant& var : shelves) {
            QVariantMap shelf = var.toMap();
            int id = shelf["shelfId"].toInt();
            QString name = shelf["name"].toString();
            int count = shelf["bookCount"].toInt();

            QString displayText = QString("%1 (%2 books)").arg(name).arg(count);
            QListWidgetItem* item = new QListWidgetItem(displayText, ui->shelfList);
            item->setData(Qt::UserRole, id);
            item->setData(Qt::UserRole + 1, name);

            m_shelvesMap[name] = id;
        }

        // انتخاب ردیف اول به صورت خودکار
        if (ui->shelfList->count() > 0 && m_currentShelfId <= 0) {
            ui->shelfList->setCurrentRow(0);
        }
    }
    // ۲. ایجاد، حذف یا تغییر نام قفسه -> دریافت مجدد لیست بروز شده
    else if (type == CommandType::CreateShelf || type == CommandType::DeleteShelf || type == CommandType::RenameShelf) {
        loadShelves();
    }
    // ۳. لیست کتاب‌های قفسه دریافت شد
    else if (type == CommandType::GetBooksInShelf) {
        ui->bookList->clear();
        clearBookDetails();

        QVariantList books = data["books"].toList();
        for (const QVariant& var : books) {
            QVariantMap book = var.toMap();
            QString title = book["title"].toString();

            QListWidgetItem* item = new QListWidgetItem(title, ui->bookList);
            item->setData(Qt::UserRole, book);
        }
    }
    // ۴. حذف کتاب از قفسه یا جابه‌جایی کتاب
    else if (type == CommandType::RemoveBookFromShelf || type == CommandType::MoveBookBetweenShelves) {
        loadBooksInShelf(m_currentShelfId);
        loadShelves(); // بروزرسانی تعداد کتاب‌های قفسه‌ها
    }
}
// ➕ اضافه کردن قفسه جدید
void ShelfManagementDialog::on_addShelfButton_clicked()
{
    bool ok;
    QString shelfName = QInputDialog::getText(this, "New Shelf",
                                              "Enter shelf name:", QLineEdit::Normal,
                                              "", &ok);
    if (ok && !shelfName.trimmed().isEmpty()) {
        int userId = SessionManager::instance()->getUserId();
        QVariantMap params;
        params["userId"] = userId;
        params["name"] = shelfName.trimmed();

        Request request(CommandType::CreateShelf, params);
        m_networkManager->sendRequest(request);
    }
}

// 🗑 حذف قفسه انتخابی
void ShelfManagementDialog::on_deleteShelfButton_clicked()
{
    if (m_currentShelfId <= 0) {
        QMessageBox::information(this, "Notice", "Please select a shelf to delete.");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirm Delete",
                                       "Are you sure you want to delete this shelf?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int userId = SessionManager::instance()->getUserId();
        QVariantMap params;
        params["userId"] = userId;
        params["shelfId"] = m_currentShelfId;

        Request request(CommandType::DeleteShelf, params);
        m_networkManager->sendRequest(request);
        m_currentShelfId = -1;
    }
}

// ✏ تغییر نام قفسه
void ShelfManagementDialog::on_renameShelfButton_clicked()
{
    QListWidgetItem* currentItem = ui->shelfList->currentItem();
    if (!currentItem || m_currentShelfId <= 0) {
        QMessageBox::information(this, "Notice", "Please select a shelf to rename.");
        return;
    }

    QString currentName = currentItem->data(Qt::UserRole + 1).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Shelf",
                                            "Enter new shelf name:", QLineEdit::Normal,
                                            currentName, &ok);
    if (ok && !newName.trimmed().isEmpty() && newName.trimmed() != currentName) {
        int userId = SessionManager::instance()->getUserId();
        QVariantMap params;
        params["userId"] = userId;
        params["shelfId"] = m_currentShelfId;
        params["newName"] = newName.trimmed();

        Request request(CommandType::RenameShelf, params);
        m_networkManager->sendRequest(request);
    }
}

// ➖ حذف کتاب از قفسه جاری
void ShelfManagementDialog::on_removeBookButton_clicked()
{
    int bookId = m_currentBookData["bookId"].toInt();
    if (m_currentShelfId <= 0 || bookId <= 0) return;
    ui->removeBookButton->setEnabled(false);

    int userId = SessionManager::instance()->getUserId();
    QVariantMap params;
    params["userId"] = userId;
    params["shelfId"] = m_currentShelfId;
    params["bookId"] = bookId;

    Request request(CommandType::RemoveBookFromShelf, params);
    m_networkManager->sendRequest(request);
}

// ➔ انتقال کتاب به قفسه‌ای دیگر
void ShelfManagementDialog::on_moveBookButton_clicked()
{
    int bookId = m_currentBookData["bookId"].toInt();
    if (m_currentShelfId <= 0 || bookId <= 0) return;

    // تهیه لیست نام قفسه‌ها (به جز قفسه جاری)
    QStringList targetShelves;
    for (auto it = m_shelvesMap.begin(); it != m_shelvesMap.end(); ++it) {
        if (it.value() != m_currentShelfId) {
            targetShelves.append(it.key());
        }
    }

    if (targetShelves.isEmpty()) {
        QMessageBox::information(this, "Move Book", "No other shelf available to move to.");
        return;
    }

    bool ok;
    QString selectedShelf = QInputDialog::getItem(this, "Move Book",
                                                  "Select target shelf:",
                                                  targetShelves, 0, false, &ok);
    if (ok && !selectedShelf.isEmpty()) {
        int toShelfId = m_shelvesMap[selectedShelf];
        int userId = SessionManager::instance()->getUserId();

        QVariantMap params;
        params["userId"] = userId;
        params["fromShelfId"] = m_currentShelfId;
        params["toShelfId"] = toShelfId;
        params["bookId"] = bookId;

        Request request(CommandType::MoveBookBetweenShelves, params);
        m_networkManager->sendRequest(request);
    }
}

// 📄 مشاهده PDF یا جزئیات بیشتر کتاب
void ShelfManagementDialog::on_readPdfButton_clicked()
{
    QString pdfPath = m_currentBookData["pdfPath"].toString();
    if (!pdfPath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));
    } else {
        QMessageBox::information(this, "PDF Reader", "PDF path is not available for this book.");
    }
}

// 📂 تغییر قفسه انتخاب‌شده در لیست
void ShelfManagementDialog::on_shelfList_currentRowChanged(int currentRow)
{
    QListWidgetItem* item = ui->shelfList->item(currentRow);
    if (item) {
        m_currentShelfId = item->data(Qt::UserRole).toInt();
        loadBooksInShelf(m_currentShelfId);
    } else {
        m_currentShelfId = -1;
        ui->bookList->clear();
        clearBookDetails();
    }
}

// 📖 تغییر کتاب انتخاب‌شده در لیست
void ShelfManagementDialog::on_bookList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (current) {
        m_currentBookData = current->data(Qt::UserRole).toMap();
        updateBookDetails(m_currentBookData);
        ui->removeBookButton->setEnabled(true);
        ui->moveBookButton->setEnabled(true);
        ui->readPdfButton->setEnabled(true);
    } else {
        clearBookDetails();
    }
}

// 🟢 به‌روزرسانی پنل جزئیات کتاب
void ShelfManagementDialog::updateBookDetails(const QVariantMap& bookData)
{
    ui->bookTitleValueLabel->setText(bookData["title"].toString());
    ui->authorValueLabel->setText("Author: " + bookData["author"].toString());
    ui->genreValueLabel->setText("Genre: " + bookData["genre"].toString());
    ui->priceValueLabel->setText(QString("Price: %1 Tooman").arg(bookData["finalPrice"].toDouble()));
    ui->descriptionTextEdit->setText(bookData["description"].toString());

    // بارگذاری تصویر کاور کتاب در صورت وجود path
    QString coverPath = bookData["coverPath"].toString();
    if (!coverPath.isEmpty()) {
        QPixmap pix(coverPath);
        if (!pix.isNull()) {
            ui->coverPreviewLabel->setPixmap(pix.scaled(ui->coverPreviewLabel->size(),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
            ui->coverPreviewLabel->setText("");
            return;
        }
    }
    ui->coverPreviewLabel->setText("No Cover");
}

// 🟢 پاکسازی پنل جزئیات
void ShelfManagementDialog::clearBookDetails()
{
    m_currentBookData.clear();
    ui->bookTitleValueLabel->setText("Select a book to view details");
    ui->authorValueLabel->setText("");
    ui->genreValueLabel->setText("");
    ui->priceValueLabel->setText("");
    ui->descriptionTextEdit->clear();
    ui->coverPreviewLabel->setText("No book selected");
    ui->coverPreviewLabel->setPixmap(QPixmap());

    ui->removeBookButton->setEnabled(false);
    ui->moveBookButton->setEnabled(false);
    ui->readPdfButton->setEnabled(false);
}



