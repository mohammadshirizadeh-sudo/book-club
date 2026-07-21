#include "applydiscountwindow.h"
#include "Publishers/ui_applydiscountwindow.h"



#include <QMessageBox>
#include <QPixmap>
#include <QDebug>

ApplyDiscountWindow::ApplyDiscountWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ApplyDiscountWindow)
{
    ui->setupUi(this);

    setupUiDefaults();
    setupConnections();

    // تست: بارگذاری اولیه کتاب‌ها
    loadBooksData();
}

ApplyDiscountWindow::~ApplyDiscountWindow()
{
    delete ui;
}

void ApplyDiscountWindow::setupUiDefaults()
{
    // تنظیم اولیه تاریخ‌ها (شروع از الان، پایان تا ۷ روز آینده)
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime().addDays(7));

    // با توجه به اینکه هر دو SpinBox در یک موقعیت مکانی قرار دارند:
    ui->percentageSpinBox->setVisible(true);
    ui->fixedAmountSpinBox->setVisible(false);

    // تنظیمات ستون‌های جدول
    ui->activeDiscountsTable->setColumnCount(6);
    QStringList headers = {"Book Title", "Original Price", "Discount", "New Price", "Type", "Valid Until"};
    ui->activeDiscountsTable->setHorizontalHeaderLabels(headers);
    ui->activeDiscountsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void ApplyDiscountWindow::setupConnections()
{
    // تغییرات مقادیر SpinBox جهت محاسبه زنده قیمت
    connect(ui->percentageSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ApplyDiscountWindow::updatePricePreview);
    connect(ui->fixedAmountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ApplyDiscountWindow::updatePricePreview);
}

void ApplyDiscountWindow::loadBooksData()
{
    ui->bookComboBox->clear();
    ui->bookComboBox->addItem("-- Select a Book --");

    // TODO: در پروژه واقعی، لیست کتاب‌ها را از دیتابیس یا سرور دریافت کنید.
    // نمونه دیتا جهت تست UI:
    m_books.clear();

    BookItem b1{1, "The Great Gatsby", 25.00, 0.0, true, false, false, QDateTime(), QDateTime(), ""};
    BookItem b2{2, "To Kill a Mockingbird", 30.00, 15.0, true, true, false, QDateTime(), QDateTime(), ""};

    m_books.append(b1);
    m_books.append(b2);

    for (const auto& book : m_books) {
        ui->bookComboBox->addItem(book.title, book.id);
    }

    refreshActiveDiscountsTable();
}

void ApplyDiscountWindow::on_bookComboBox_currentIndexChanged(int index)
{
    if (index <= 0) {
        clearForm();
        return;
    }

    int bookIndex = index - 1; // به دلیل آیتم اول "-- Select a Book --"
    const BookItem& selectedBook = m_books[bookIndex];

    ui->currentPriceLabel->setText(QString("Current Price: $%1").arg(selectedBook.originalPrice, 0, 'f', 2));

    if (selectedBook.hasDiscount) {
        QString unit = selectedBook.isPercentage ? "%" : "$";
        ui->currentDiscountLabel->setText(QString("Current Discount: %1%2")
                                              .arg(selectedBook.currentDiscount, 0, 'f', 1)
                                              .arg(unit));
    } else {
        ui->currentDiscountLabel->setText("Current Discount: None");
    }

    ui->bookInfoLabel->setText(QString("Selected: %1 | Price: $%2")
                                   .arg(selectedBook.title)
                                   .arg(selectedBook.originalPrice, 0, 'f', 2));

    // نمایش کاور در صورت وجود
    if (!selectedBook.coverPath.isEmpty()) {
        ui->bookCoverPreviewLabel->setPixmap(QPixmap(selectedBook.coverPath).scaled(ui->bookCoverPreviewLabel->size(), Qt::KeepAspectRatio));
    } else {
        ui->bookCoverPreviewLabel->setText("Cover");
    }

    updatePricePreview();
}

void ApplyDiscountWindow::on_percentageRadio_toggled(bool checked)
{
    // سوئیچ بین درصد و مبلغ ثابت
    ui->percentageSpinBox->setVisible(checked);
    ui->percentageSpinBox->setEnabled(checked);

    ui->fixedAmountSpinBox->setVisible(!checked);
    ui->fixedAmountSpinBox->setEnabled(!checked);

    updatePricePreview();
}

void ApplyDiscountWindow::on_timedDiscountCheck_toggled(bool checked)
{
    ui->startDateEdit->setEnabled(checked);
    ui->endDateEdit->setEnabled(checked);
}

void ApplyDiscountWindow::updatePricePreview()
{
    int index = ui->bookComboBox->currentIndex();
    if (index <= 0) {
        ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
        return;
    }

    double originalPrice = m_books[index - 1].originalPrice;
    double discountValue = 0.0;
    double newPrice = originalPrice;
    double savings = 0.0;

    if (ui->percentageRadio->isChecked()) {
        discountValue = ui->percentageSpinBox->value();
        savings = originalPrice * (discountValue / 100.0);
    } else {
        discountValue = ui->fixedAmountSpinBox->value();
        savings = discountValue;
    }

    newPrice = originalPrice - savings;
    if (newPrice < 0) newPrice = 0.0;

    ui->newPricePreviewLabel->setText(QString("New Price: $%1 (Save: $%2)")
                                          .arg(newPrice, 0, 'f', 2)
                                          .arg(savings, 0, 'f', 2));
}

void ApplyDiscountWindow::on_applyDiscountButton_clicked()
{
    int index = ui->bookComboBox->currentIndex();
    if (index <= 0) {
        QMessageBox::warning(this, "Warning", "Please select a book first!");
        return;
    }

    BookItem& selectedBook = m_books[index - 1];

    if (ui->timedDiscountCheck->isChecked()) {
        if (ui->endDateEdit->dateTime() <= ui->startDateEdit->dateTime()) {
            QMessageBox::warning(this, "Invalid Date", "End date must be after start date!");
            return;
        }
        selectedBook.isTimed = true;
        selectedBook.startDate = ui->startDateEdit->dateTime();
        selectedBook.endDate = ui->endDateEdit->dateTime();
    } else {
        selectedBook.isTimed = false;
    }

    selectedBook.hasDiscount = true;
    selectedBook.isPercentage = ui->percentageRadio->isChecked();
    selectedBook.currentDiscount = selectedBook.isPercentage ? ui->percentageSpinBox->value()
                                                           : ui->fixedAmountSpinBox->value();

    // TODO: ارسال درخواست شبکه یا ثبت در دیتابیس
    /*
    QVariantMap params;
    params["bookId"] = selectedBook.id;
    params["discountValue"] = selectedBook.discountValue;
    params["isPercentage"] = selectedBook.isPercentage;
    ...
    */

    QMessageBox::information(this, "Success", "Discount applied successfully!");

    // بروزرسانی UI و جدول
    on_bookComboBox_currentIndexChanged(index);
    refreshActiveDiscountsTable();
}

void ApplyDiscountWindow::on_removeDiscountButton_clicked()
{
    int index = ui->bookComboBox->currentIndex();
    if (index <= 0) {
        QMessageBox::warning(this, "Warning", "Please select a book first!");
        return;
    }

    BookItem& selectedBook = m_books[index - 1];

    if (!selectedBook.hasDiscount) {
        QMessageBox::information(this, "Notice", "This book has no active discount.");
        return;
    }

    selectedBook.hasDiscount = false;
    selectedBook.currentDiscount = 0.0;
    selectedBook.isTimed = false;

    // TODO: ارسال درخواست حذف تخفیف به سرور / دیتابیس

    QMessageBox::information(this, "Success", "Discount removed successfully!");

    on_bookComboBox_currentIndexChanged(index);
    refreshActiveDiscountsTable();
}

void ApplyDiscountWindow::refreshActiveDiscountsTable()
{
    ui->activeDiscountsTable->setRowCount(0);

    int row = 0;
    for (const auto& book : m_books) {
        if (!book.hasDiscount) continue;

        ui->activeDiscountsTable->insertRow(row);

        double savings = book.isPercentage ? (book.originalPrice * book.currentDiscount / 100.0) : book.currentDiscount;
        double newPrice = book.originalPrice - savings;

        ui->activeDiscountsTable->setItem(row, 0, new QTableWidgetItem(book.title));
        ui->activeDiscountsTable->setItem(row, 1, new QTableWidgetItem(QString("$%1").arg(book.originalPrice, 0, 'f', 2)));
        ui->activeDiscountsTable->setItem(row, 2, new QTableWidgetItem(QString("%1%2").arg(book.currentDiscount).arg(book.isPercentage ? "%" : "$")));
        ui->activeDiscountsTable->setItem(row, 3, new QTableWidgetItem(QString("$%1").arg(newPrice, 0, 'f', 2)));
        ui->activeDiscountsTable->setItem(row, 4, new QTableWidgetItem(book.isTimed ? "Timed" : "Permanent"));
        ui->activeDiscountsTable->setItem(row, 5, new QTableWidgetItem(book.isTimed ? book.endDate.toString("yyyy-MM-dd HH:mm") : "N/A"));

        row++;
    }
}

void ApplyDiscountWindow::on_clearFormButton_clicked()
{
    clearForm();
}

void ApplyDiscountWindow::clearForm()
{
    ui->bookComboBox->setCurrentIndex(0);
    ui->percentageRadio->setChecked(true);
    ui->percentageSpinBox->setValue(10);
    ui->fixedAmountSpinBox->setValue(1.0);
    ui->timedDiscountCheck->setChecked(false);
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime().addDays(7));
    ui->bookCoverPreviewLabel->setText("Cover");
    ui->bookInfoLabel->setText("Select a book from the dropdown above to view its details and apply discount.");
    ui->currentPriceLabel->setText("Current Price: $0.00");
    ui->currentDiscountLabel->setText("Current Discount: None");
    ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
}

void ApplyDiscountWindow::on_quitPushButton_clicked()
{
    this->close();
}