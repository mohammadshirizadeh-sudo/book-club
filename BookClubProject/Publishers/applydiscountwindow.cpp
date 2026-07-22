#include "applydiscountwindow.h"
#include "Publishers/ui_applydiscountwindow.h"
#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"

#include <QMessageBox>
#include <QPixmap>
#include <QDateTime>

ApplyDiscountWindow::ApplyDiscountWindow(NetworkManager* networkManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApplyDiscountWindow),
    m_networkManager(networkManager)
{
    ui->setupUi(this);

    // اتصال سیگنال شبکه به پردازشگر پاسخ‌ها
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ApplyDiscountWindow::handleResponse);

    // تنظیم مقادیر اولیه تاریخ‌ها (شروع از الان، پایان تا ۷ روز آینده)
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime().addDays(7));

    // تنظیم حالت اولیه فرم
    clearForm();
}

ApplyDiscountWindow::~ApplyDiscountWindow()
{
    // قطع اتصال برای جلوگیری از Memory Leak و پردازش‌های اضافه
    disconnect(m_networkManager, &NetworkManager::responseReceived,
               this, &ApplyDiscountWindow::handleResponse);
    delete ui;
}

void ApplyDiscountWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadPublisherBooks();
}

// 🟢 درخواست بارگذاری کتاب‌های مربوط به ناشر جارى
void ApplyDiscountWindow::loadPublisherBooks()
{
    int publisherId = SessionManager::instance()->getUserId();
    if (publisherId <= 0) return;

    QVariantMap params;
    params["publisherId"] = publisherId;

    // ارسال دستور دریافت کتاب‌های ناشر
    Request request(CommandType::GetPublisherBooks, params);
    m_networkManager->sendRequest(request);
}

// 🟢 مدیریت مرکزی پاسخ‌های دریافتی از سرور
void ApplyDiscountWindow::handleResponse(const Response& response)
{
    CommandType type = response.getCommandType();

    // فیلتر کردن دستورات مرتبط با این صفحه
    if (type != CommandType::GetPublisherBooks &&
        type != CommandType::ApplyDiscount &&
        type != CommandType::RemoveDiscount) {
        return;
    }

    if (!response.isSuccess()) {
        QMessageBox::warning(this, "Error", response.getMessage());
        return;
    }

    QVariantMap data = response.getData();

    // ۱. دریافت لیست کتاب‌های ناشر
    if (type == CommandType::GetPublisherBooks) {
        ui->bookComboBox->blockSignals(true);
        ui->bookComboBox->clear();
        ui->bookComboBox->addItem("-- Select a Book --", -1);

        m_publisherBooks.clear();
        QVariantList books = data["books"].toList();

        for (const QVariant& var : books) {
            QVariantMap book = var.toMap();
            m_publisherBooks.append(book);

            int id = book["bookId"].toInt();
            QString title = book["title"].toString();
            ui->bookComboBox->addItem(title, id);
        }
        ui->bookComboBox->blockSignals(false);

        // بروزرسانی جدول تخفیف‌های فعال
        updateActiveDiscountsTable();
    }
    // ۲. اعمال یا حذف موفقیت‌آمیز تخفیف
    else if (type == CommandType::ApplyDiscount || type == CommandType::RemoveDiscount) {
        QMessageBox::information(this, "Success", response.getMessage());
        clearForm();
        loadPublisherBooks(); // بروزرسانی اطلاعات از سرور
    }
}

// 🔄 تغییر کتاب انتخابی از کمبوباکس
void ApplyDiscountWindow::on_bookComboBox_currentIndexChanged(int index)
{
    if (index <= 0) {
        clearForm();
        return;
    }

    m_selectedBookId = ui->bookComboBox->currentData().toInt();

    // پیدا کردن اطلاعات کتاب در لیست
    for (const auto& book : m_publisherBooks) {
        if (book["bookId"].toInt() == m_selectedBookId) {
            updateBookDetailsUI(book);
            break;
        }
    }
}

// 📖 بروزرسانی نمایش جزئیات کتاب انتخابی
void ApplyDiscountWindow::updateBookDetailsUI(const QVariantMap& bookData)
{
    double price = bookData["price"].toDouble();
    double discountPercent = bookData["discountPercent"].toDouble();
    QString author = bookData["author"].toString();

    ui->bookInfoLabel->setText(QString("Author: %1 | Base Price: $%2").arg(author).arg(price, 0, 'f', 2));
    ui->currentPriceLabel->setText(QString("Current Price: $%1").arg(bookData["finalPrice"].toDouble(), 0, 'f', 2));

    if (discountPercent > 0) {
        ui->currentDiscountLabel->setText(QString("Current Discount: %1%").arg(discountPercent));
    } else {
        ui->currentDiscountLabel->setText("Current Discount: None");
    }

    // کاور کتاب
    QString coverPath = bookData["coverPath"].toString();
    if (!coverPath.isEmpty()) {
        QPixmap pix(coverPath);
        if (!pix.isNull()) {
            ui->bookCoverPreviewLabel->setPixmap(pix.scaled(ui->bookCoverPreviewLabel->size(),
                                                            Qt::KeepAspectRatio,
                                                            Qt::SmoothTransformation));
            ui->bookCoverPreviewLabel->setText("");
        } else {
            ui->bookCoverPreviewLabel->setText("No Cover");
        }
    } else {
        ui->bookCoverPreviewLabel->setText("No Cover");
    }

    updatePricePreview();
}

// 💰 مدیریت سویچ بین درصد و مبلغ ثابت
void ApplyDiscountWindow::on_percentageRadio_toggled(bool checked)
{
    ui->percentageSpinBox->setEnabled(checked);
    ui->fixedAmountSpinBox->setEnabled(!checked);
    updatePricePreview();
}

void ApplyDiscountWindow::on_fixedAmountRadio_toggled(bool checked)
{
    ui->fixedAmountSpinBox->setEnabled(checked);
    ui->percentageSpinBox->setEnabled(!checked);
    updatePricePreview();
}

// ⏰ فعال/غیرفعال‌سازی تاریخ زمان‌دار
void ApplyDiscountWindow::on_timedDiscountCheck_toggled(bool checked)
{
    ui->startDateEdit->setEnabled(checked);
    ui->endDateEdit->setEnabled(checked);
}

// 🧮 محاسبه آنلاین پیش‌نمایش قیمت جدید
void ApplyDiscountWindow::on_percentageSpinBox_valueChanged(int value)
{
    Q_UNUSED(value);
    updatePricePreview();
}

void ApplyDiscountWindow::on_fixedAmountSpinBox_valueChanged(double value)
{
    Q_UNUSED(value);
    updatePricePreview();
}

void ApplyDiscountWindow::updatePricePreview()
{
    if (m_selectedBookId <= 0) {
        ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
        return;
    }

    double originalPrice = 0.0;
    for (const auto& book : m_publisherBooks) {
        if (book["bookId"].toInt() == m_selectedBookId) {
            originalPrice = book["price"].toDouble();
            break;
        }
    }

    double newPrice = originalPrice;
    double savings = 0.0;

    if (ui->percentageRadio->isChecked()) {
        double percent = ui->percentageSpinBox->value();
        savings = originalPrice * (percent / 100.0);
        newPrice = originalPrice - savings;
    } else {
        savings = ui->fixedAmountSpinBox->value();
        newPrice = originalPrice - savings;
    }

    if (newPrice < 0) newPrice = 0;

    ui->newPricePreviewLabel->setText(QString("New Price: $%1 (Save: $%2)")
                                          .arg(newPrice, 0, 'f', 2)
                                          .arg(savings, 0, 'f', 2));
}

// ✅ ارسال درخواست اعمال تخفیف به سرور
void ApplyDiscountWindow::on_applyDiscountButton_clicked()
{
    if (m_selectedBookId <= 0) {
        QMessageBox::warning(this, "Warning", "Please select a book first.");
        return;
    }

    int publisherId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["publisherId"] = publisherId;
    params["bookId"] = m_selectedBookId;

    if (ui->percentageRadio->isChecked()) {
        params["discountType"] = "percentage";
        params["discountValue"] = ui->percentageSpinBox->value();
    } else {
        params["discountType"] = "fixed";
        params["discountValue"] = ui->fixedAmountSpinBox->value();
    }

    bool isTimed = ui->timedDiscountCheck->isChecked();
    params["isTimed"] = isTimed;

    if (isTimed) {
        params["startDate"] = ui->startDateEdit->dateTime().toString(Qt::ISODate);
        params["endDate"] = ui->endDateEdit->dateTime().toString(Qt::ISODate);
    } else {
        params["startDate"] = "";
        params["endDate"] = "";
    }

    Request request(CommandType::ApplyDiscount, params);
    m_networkManager->sendRequest(request);
}

// ❌ ارسال درخواست حذف تخفیف به سرور
void ApplyDiscountWindow::on_removeDiscountButton_clicked()
{
    if (m_selectedBookId <= 0) {
        QMessageBox::warning(this, "Warning", "Please select a book first.");
        return;
    }

    int publisherId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["publisherId"] = publisherId;
    params["bookId"] = m_selectedBookId;

    Request request(CommandType::RemoveDiscount, params);
    m_networkManager->sendRequest(request);
}

// 🔄 پر کردن جدول تخفیف‌های فعال
void ApplyDiscountWindow::updateActiveDiscountsTable()
{
    ui->activeDiscountsTable->clearContents();
    ui->activeDiscountsTable->setRowCount(0);

    int row = 0;
    for (const auto& book : m_publisherBooks) {
        double discountPercent = book["discountPercent"].toDouble();

        // نمایش کتاب‌هایی که دارای تخفیف فعال هستند
        if (discountPercent > 0 || book["isDiscounted"].toBool()) {
            ui->activeDiscountsTable->insertRow(row);

            ui->activeDiscountsTable->setItem(row, 0, new QTableWidgetItem(book["title"].toString()));
            ui->activeDiscountsTable->setItem(row, 1, new QTableWidgetItem(QString("$%1").arg(book["price"].toDouble(), 0, 'f', 2)));
            ui->activeDiscountsTable->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(discountPercent)));
            ui->activeDiscountsTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(book["finalPrice"].toDouble(), 0, 'f', 2)));

            QString typeStr = book["isTimed"].toBool() ? "Timed" : "Permanent";
            ui->activeDiscountsTable->setItem(row, 4, new QTableWidgetItem(typeStr));

            QString endDateStr = book["endDate"].toString();
            ui->activeDiscountsTable->setItem(row, 5, new QTableWidgetItem(endDateStr.isEmpty() ? "N/A" : endDateStr));

            row++;
        }
    }
}

// 🔄 پاکسازی فرم
void ApplyDiscountWindow::on_clearFormButton_clicked()
{
    clearForm();
}

void ApplyDiscountWindow::clearForm()
{
    m_selectedBookId = -1;
    ui->bookComboBox->setCurrentIndex(0);
    ui->bookInfoLabel->setText("Select a book from the dropdown above to view its details and apply discount.");
    ui->currentPriceLabel->setText("Current Price: $0.00");
    ui->currentDiscountLabel->setText("Current Discount: None");
    ui->bookCoverPreviewLabel->setText("Cover");
    ui->bookCoverPreviewLabel->setPixmap(QPixmap());

    ui->percentageRadio->setChecked(true);
    ui->percentageSpinBox->setValue(10);
    ui->fixedAmountSpinBox->setValue(1.0);

    ui->timedDiscountCheck->setChecked(false);
    ui->startDateEdit->setEnabled(false);
    ui->endDateEdit->setEnabled(false);

    ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
}

// ⬅ بازگشت/خروج
void ApplyDiscountWindow::on_quitPushButton_clicked()
{
    this->close();
}