#include "editbookswindow.h"
#include "Publishers/ui_editbookswindow.h"
#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

EditBooksWindow::EditBooksWindow(NetworkManager* networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditBooksWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    // اتصال سیگنال دریافت پاسخ از سرور
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &EditBooksWindow::handleResponse);

    // غیرفعال کردن اولیه فرم تا زمان انتخاب کتاب
    clearForm();
}

EditBooksWindow::~EditBooksWindow()
{
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &EditBooksWindow::handleResponse);
    delete ui;
}

void EditBooksWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadPublisherBooks();
}

// 🟢 درخواست بارگذاری لیست کتاب‌های ناشر
void EditBooksWindow::loadPublisherBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    if (publisherId <= 0) return;

    ui->statusLabel->setText("Loading books list...");

    QVariantMap params;
    params["publisherId"] = publisherId;

    Request request(CommandType::GetPublisherBooks, params);
    m_networkManager->sendRequest(request);
}

// 🟢 دریافت اطلاعات تکمیلی کتاب با ID مشخص
void EditBooksWindow::loadBookDetails(int bookId)
{
    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["bookId"] = bookId;
    params["userId"] = userId;

    Request request(CommandType::GetBookById, params);
    m_networkManager->sendRequest(request);
}

// 🟢 مدیریت مرکزی پاسخ‌های سرور
void EditBooksWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // فیلتر پاسخ‌های غیرمرتبط
    if (type != CommandType::GetPublisherBooks &&
        type != CommandType::GetBookById &&
        type != CommandType::EditBook) {
        return;
    }

    if (!response.isSuccess()) {
        ui->statusLabel->setText("Error: " + response.getMessage());
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    // ۱. لیست کتاب‌های ناشر دریافت شد
    if (type == CommandType::GetPublisherBooks) {
        ui->booksListWidget->clear();
        clearForm();

        QVariantList books = data["books"].toList();
        if (books.isEmpty()) {
            ui->statusLabel->setText("No books found.");
            return;
        }

        for (const QVariant& var : books) {
            QVariantMap book = var.toMap();
            int id = book["bookId"].toInt();
            QString title = book["title"].toString();
            QString author = book["author"].toString();

            QString itemText = QString("%1\nBy: %2").arg(title, author);
            QListWidgetItem* item = new QListWidgetItem(itemText, ui->booksListWidget);
            item->setData(Qt::UserRole, id);
        }

        ui->statusLabel->setText("Select a book from the list to begin editing...");
    }
    // ۲. اطلاعات کامل کتاب جهت ویرایش دریافت شد
    else if (type == CommandType::GetBookById) {
        m_currentBookData = data;
        populateForm(m_currentBookData);
        setFormEnabled(true);
        ui->statusLabel->setText("Editing: " + data["title"].toString());
    }
    // ۳. تغییرات کتاب با موفقیت ذخیره شد
    else if (type == CommandType::EditBook) {
        QMessageBox::information(this, "Success", response.getMessage());
        loadPublisherBooks(); // بروزرسانی لیست
    }
}

// 📖 تغییر کتاب انتخاب‌شده در ListWidget
void EditBooksWindow::on_booksListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (current) {
        m_selectedBookId = current->data(Qt::UserRole).toInt();
        loadBookDetails(m_selectedBookId);
    } else {
        clearForm();
    }
}

// 🔍 جستجو در لیست کتاب‌ها
void EditBooksWindow::on_searchBookLineEdit_textChanged(const QString &arg1)
{
    QString filter = arg1.trimmed().toLower();
    for (int i = 0; i < ui->booksListWidget->count(); ++i) {
        QListWidgetItem* item = ui->booksListWidget->item(i);
        bool matches = item->text().toLower().contains(filter);
        item->setHidden(!matches);
    }
}

// 🔄 دکمه Refresh لیست
void EditBooksWindow::on_refreshListBtn_clicked()
{
    loadPublisherBooks();
}

// ✏️ پر کردن فیلدهای فرم با اطلاعات کتاب
void EditBooksWindow::populateForm(const QVariantMap& bookData)
{
    ui->titleLineEdit->setText(bookData["title"].toString());
    ui->authorLineEdit->setText(bookData["author"].toString());

    // تنظیم ژانر در ComboBox
    QString genreStr = bookData["genre"].toString();
    int genreIndex = ui->genreComboBox->findText(genreStr, Qt::MatchFixedString);
    if (genreIndex >= 0) {
        ui->genreComboBox->setCurrentIndex(genreIndex);
    } else {
        ui->genreComboBox->setCurrentIndex(0);
    }

    ui->descriptionTextEdit->setText(bookData["description"].toString());
    ui->priceSpinBox->setValue(bookData["price"].toDouble());

    // کاور و فایل PDF
    m_selectedCoverPath = bookData["coverPath"].toString();
    updateCoverPreview(m_selectedCoverPath);

    m_selectedPdfPath = bookData["pdfPath"].toString();
    ui->pdfPathLineEdit->setText(m_selectedPdfPath.isEmpty() ? "No PDF attached" : m_selectedPdfPath);
}

// 📷 آپلود کاور جدید
void EditBooksWindow::on_uploadCoverButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Cover Image", "",
                                                    "Image Files (*.png *.jpg *.jpeg)");
    if (!filePath.isEmpty()) {
        m_selectedCoverPath = filePath;
        updateCoverPreview(m_selectedCoverPath);
    }
}

// 🗑️ حذف کاور
void EditBooksWindow::on_removeCoverButton_clicked()
{
    m_selectedCoverPath = "";
    updateCoverPreview("");
}

// 📄 آپلود فایل PDF جدید
void EditBooksWindow::on_uploadPdfButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select PDF File", "",
                                                    "PDF Files (*.pdf)");
    if (!filePath.isEmpty()) {
        m_selectedPdfPath = filePath;
        ui->pdfPathLineEdit->setText(m_selectedPdfPath);
    }
}

// 🔄 نمایش پیش‌نمایش تصویر کاور
void EditBooksWindow::updateCoverPreview(const QString& coverPath)
{
    if (!coverPath.isEmpty()) {
        QPixmap pix(coverPath);
        if (!pix.isNull()) {
            ui->coverPlaceholderLabel->setPixmap(pix.scaled(ui->coverPlaceholderLabel->size(),
                                                            Qt::KeepAspectRatio,
                                                            Qt::SmoothTransformation));
            ui->coverTextLabel->setText("Cover Loaded");
            return;
        }
    }

    ui->coverPlaceholderLabel->setPixmap(QPixmap());
    ui->coverPlaceholderLabel->setText("📖");
    ui->coverTextLabel->setText("No image selected");
}

// 💾 ذخیره تغییرات
void EditBooksWindow::on_saveChangesButton_clicked()
{
    if (m_selectedBookId <= 0) {
        QMessageBox::warning(this, "Warning", "Please select a book first.");
        return;
    }

    QString title = ui->titleLineEdit->text().trimmed();
    QString author = ui->authorLineEdit->text().trimmed();
    QString genre = ui->genreComboBox->currentText();
    QString description = ui->descriptionTextEdit->toPlainText().trimmed();
    double price = ui->priceSpinBox->value();

    if (title.isEmpty() || author.isEmpty() || ui->genreComboBox->currentIndex() <= 0) {
        QMessageBox::warning(this, "Validation Error", "Please fill in Title, Author, and Genre.");
        return;
    }

    QVariantMap params;
    params["bookId"] = m_selectedBookId;
    params["title"] = title;
    params["author"] = author;
    params["genre"] = genre;
    params["description"] = description;
    params["price"] = price;

    // حفظ مقدار درصد تخفیف قبلی کتاب
    params["discount"] = m_currentBookData["discountPercent"].toDouble();
    params["coverPath"] = m_selectedCoverPath;
    params["pdfPath"] = m_selectedPdfPath;

    Request request(CommandType::EditBook, params);
    m_networkManager->sendRequest(request);
}

// 🔄 بازگردانی فرم به حالت اول
void EditBooksWindow::on_resetFormButton_clicked()
{
    if (!m_currentBookData.isEmpty()) {
        populateForm(m_currentBookData);
    }
}

// 🟢 فعال/غیرفعال‌سازی عناصر فرم
void EditBooksWindow::setFormEnabled(bool enabled)
{
    ui->titleLineEdit->setEnabled(enabled);
    ui->authorLineEdit->setEnabled(enabled);
    ui->genreComboBox->setEnabled(enabled);
    ui->descriptionTextEdit->setEnabled(enabled);
    ui->priceSpinBox->setEnabled(enabled);
    ui->uploadCoverButton->setEnabled(enabled);
    ui->removeCoverButton->setEnabled(enabled);
    ui->uploadPdfButton->setEnabled(enabled);
    ui->saveChangesButton->setEnabled(enabled);
    ui->resetFormButton->setEnabled(enabled);
}

// 🟢 پاکسازی فرم
void EditBooksWindow::clearForm()
{
    m_selectedBookId = -1;
    m_currentBookData.clear();
    m_selectedCoverPath.clear();
    m_selectedPdfPath.clear();

    ui->titleLineEdit->clear();
    ui->authorLineEdit->clear();
    ui->genreComboBox->setCurrentIndex(0);
    ui->descriptionTextEdit->clear();
    ui->priceSpinBox->setValue(19.99);
    ui->pdfPathLineEdit->clear();

    updateCoverPreview("");
    setFormEnabled(false);

    ui->statusLabel->setText("Select a book from the list to begin editing...");
}

// ⬅ بازگشت/بستن پنجره
void EditBooksWindow::on_quitPushButton_clicked()
{
    this->close();
}