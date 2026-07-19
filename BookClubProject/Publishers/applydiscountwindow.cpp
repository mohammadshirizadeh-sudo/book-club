#include "applydiscountwindow.h"
#include "Publishers/ui_applydiscountwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

ApplyDiscountWindow::ApplyDiscountWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApplyDiscountWindow),
    m_currentBookId(-1),
    m_currentBookPrice(0.0)
{
    ui->setupUi(this);

    // Initialize UI
    loadBooksList();
    loadActiveDiscounts();

    // Set default dates for timed discount
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime().addDays(7));

    // Initially hide fixed amount spinbox
    ui->fixedAmountSpinBox->setVisible(false);
}

ApplyDiscountWindow::~ApplyDiscountWindow()
{
    delete ui;
}

void ApplyDiscountWindow::loadBooksList()
{
    // Clear existing items
    ui->bookComboBox->clear();

    // Add placeholder
    ui->bookComboBox->addItem("-- Select a Book --", -1);

    // Load books from data source
    QStringList books = getPublisherBooks();

    for (int i = 0; i < books.size(); i++) {
        // Assuming format: "ID|Title"
        QStringList parts = books[i].split("|");
        if (parts.size() >= 2) {
            int bookId = parts[0].toInt();
            QString bookTitle = parts[1];
            ui->bookComboBox->addItem(bookTitle, bookId);
        } else {
            ui->bookComboBox->addItem(books[i], i);
        }
    }
}

void ApplyDiscountWindow::updateBookInfo(int bookIndex)
{
    if (bookIndex <= 0) {
        // No book selected or placeholder selected
        m_currentBookId = -1;
        m_currentBookPrice = 0.0;
        m_currentBookTitle.clear();

        ui->currentPriceLabel->setText("Current Price: $0.00");
        ui->currentDiscountLabel->setText("Current Discount: None");
        ui->bookCoverPreviewLabel->setText("Cover");
        ui->bookInfoLabel->setText("Select a book from the dropdown above to view its details and apply discount.");
        ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
        return;
    }

    // Get book ID from combo box data
    m_currentBookId = ui->bookComboBox->currentData().toInt();
    m_currentBookTitle = ui->bookComboBox->currentText();

    // Get price from database
    m_currentBookPrice = getBookPrice(m_currentBookId);

    // Update UI with book info
    ui->currentPriceLabel->setText(QString("Current Price: $%1").arg(m_currentBookPrice, 0, 'f', 2));

    // TODO: Check if book has active discount
    ui->currentDiscountLabel->setText("Current Discount: None");

    // Update book info label
    QString infoText = QString("<b>%1</b><br>Author: Sample Author | Genre: Fiction | Published: 2024")
                           .arg(m_currentBookTitle);
    ui->bookInfoLabel->setText(infoText);
    ui->bookCoverPreviewLabel->setText("📖");

    // Update new price preview
    updateNewPricePreview();
}

void ApplyDiscountWindow::updateNewPricePreview()
{
    if (m_currentBookId == -1 || m_currentBookPrice <= 0) {
        ui->newPricePreviewLabel->setText("New Price: $0.00 (Save: $0.00)");
        return;
    }

    double discountValue = 0;
    double newPrice = m_currentBookPrice;
    double savings = 0;

    if (ui->percentageRadio->isChecked()) {
        discountValue = ui->percentageSpinBox->value();
        savings = m_currentBookPrice * (discountValue / 100.0);
        newPrice = m_currentBookPrice - savings;
    } else if (ui->fixedAmountRadio->isChecked()) {
        discountValue = ui->fixedAmountSpinBox->value();
        savings = qMin(discountValue, m_currentBookPrice);
        newPrice = m_currentBookPrice - savings;
    }

    // Ensure price doesn't go negative
    newPrice = qMax(0.01, newPrice);

    ui->newPricePreviewLabel->setText(
        QString("New Price: $%1 (Save: $%2)")
            .arg(newPrice, 0, 'f', 2)
            .arg(savings, 0, 'f', 2)
        );
}

bool ApplyDiscountWindow::validateInputs()
{
    // Check if a book is selected
    if (m_currentBookId == -1) {
        QMessageBox::warning(this, "Validation Error",
                             "Please select a book first!");
        return false;
    }

    // Validate discount value
    if (ui->percentageRadio->isChecked()) {
        if (ui->percentageSpinBox->value() <= 0 ||
            ui->percentageSpinBox->value() > 99) {
            QMessageBox::warning(this, "Validation Error",
                                 "Please enter a valid percentage (1-99%)!");
            return false;
        }
    }

    // Validate timed discount dates if enabled
    if (ui->timedDiscountCheck->isChecked()) {
        QDateTime startDate = ui->startDateEdit->dateTime();
        QDateTime endDate = ui->endDateEdit->dateTime();

        if (startDate >= endDate) {
            QMessageBox::warning(this, "Validation Error",
                                 "End date must be after start date!");
            return false;
        }

        if (startDate < QDateTime::currentDateTime()) {
            QMessageBox::warning(this, "Validation Error",
                                 "Start date cannot be in the past!");
            return false;
        }
    }

    return true;
}

void ApplyDiscountWindow::applyDiscountToBook()
{
    if (!validateInputs()) {
        return;
    }

    QString discountType;
    double discountValue;

    if (ui->percentageRadio->isChecked()) {
        discountType = "Percentage";
        discountValue = ui->percentageSpinBox->value();
    } else {
        discountType = "Fixed";
        discountValue = ui->fixedAmountSpinBox->value();
    }

    QDateTime startDate, endDate;

    if (ui->timedDiscountCheck->isChecked()) {
        startDate = ui->startDateEdit->dateTime();
        endDate = ui->endDateEdit->dateTime();
    }

    // Save to database
    bool success = saveDiscountToDatabase(m_currentBookId, discountType,
                                          discountValue, startDate, endDate);

    if (success) {
        QString message;
        if (ui->timedDiscountCheck->isChecked()) {
            message = QString("Discount applied successfully!\n\n"
                              "Book: %1\n"
                              "Discount: %2%3\n"
                              "Valid until: %4")
                          .arg(m_currentBookTitle)
                          .arg(discountType == "Percentage" ?
                                   QString("%1").arg(discountValue) : "$" + QString::number(discountValue))
                          .arg(discountType == "Percentage" ? "%" : "")
                          .arg(endDate.toString("yyyy-MM-dd HH:mm"));
        } else {
            message = QString("Discount applied successfully!\n\n"
                              "Book: %1\n"
                              "Discount: %2%3\n"
                              "This is a permanent discount.")
                          .arg(m_currentBookTitle)
                          .arg(discountType == "Percentage" ?
                                   QString("%1").arg(discountValue) : "$" + QString::number(discountValue))
                          .arg(discountType == "Percentage" ? "%" : "");
        }

        QMessageBox::information(this, "Success", message);

        // Refresh active discounts table
        loadActiveDiscounts();

        // Update current discount display
        if (discountType == "Percentage") {
            ui->currentDiscountLabel->setText(
                QString("Current Discount: %1% OFF").arg(discountValue));
        } else {
            ui->currentDiscountLabel->setText(
                QString("Current Discount: $%1 OFF").arg(discountValue, 0, 'f', 2));
        }
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to apply discount. Please try again.");
    }
}

void ApplyDiscountWindow::removeDiscountFromBook()
{
    if (m_currentBookId == -1) {
        QMessageBox::warning(this, "Warning",
                             "Please select a book first!");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Discount",
                                  QString("Are you sure you want to remove the discount from \"%1\"?")
                                      .arg(m_currentBookTitle),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool success = removeDiscountFromDatabase(m_currentBookId);

        if (success) {
            QMessageBox::information(this, "Success",
                                     "Discount removed successfully!\nThe price has been restored.");
            ui->currentDiscountLabel->setText("Current Discount: None");
            loadActiveDiscounts();
        } else {
            QMessageBox::critical(this, "Error",
                                  "Failed to remove discount. Please try again.");
        }
    }
}

void ApplyDiscountWindow::clearForm()
{
    ui->bookComboBox->setCurrentIndex(0);
    ui->percentageRadio->setChecked(true);
    ui->percentageSpinBox->setValue(10);
    ui->fixedAmountSpinBox->setValue(1.00);
    ui->timedDiscountCheck->setChecked(false);
    ui->startDateEdit->setDateTime(QDateTime::currentDateTime());
    ui->endDateEdit->setDateTime(QDateTime::currentDateTime().addDays(7));
    updateNewPricePreview();
}

void ApplyDiscountWindow::loadActiveDiscounts()
{
    // Clear table
    ui->activeDiscountsTable->setRowCount(0);

    // TODO: Load actual discounts from database
    // Sample data for demonstration
    int sampleCount = 3;
    ui->activeDiscountsTable->setRowCount(sampleCount);

    struct DiscountData {
        QString title;
        QString originalPrice;
        QString discount;
        QString newPrice;
        QString type;
        QString validUntil;
    };

    DiscountData samples[] = {
        {"The Great Adventure", "$29.99", "15%", "$25.49", "%", "Permanent"},
        {"Mystery of Shadows", "$24.99", "$5.00", "$19.99", "$", "2025-02-28"},
        {"Science Today", "$34.99", "20%", "$27.99", "%", "2025-03-15"}
    };

    for (int row = 0; row < sampleCount; row++) {
        QTableWidgetItem *titleItem = new QTableWidgetItem(samples[row].title);
        titleItem->setFont(QFont("Segoe UI", 10));
        ui->activeDiscountsTable->setItem(row, 0, titleItem);

        QTableWidgetItem *origItem = new QTableWidgetItem(samples[row].originalPrice);
        origItem->setTextAlignment(Qt::AlignCenter);
        ui->activeDiscountsTable->setItem(row, 1, origItem);

        QTableWidgetItem *discItem = new QTableWidgetItem(samples[row].discount);
        discItem->setTextAlignment(Qt::AlignCenter);
        discItem->setForeground(QColor(255, 87, 34));
        discItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->activeDiscountsTable->setItem(row, 2, discItem);

        QTableWidgetItem *newItem = new QTableWidgetItem(samples[row].newPrice);
        newItem->setTextAlignment(Qt::AlignCenter);
        newItem->setForeground(QColor(76, 175, 80));
        newItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        ui->activeDiscountsTable->setItem(row, 3, newItem);

        QTableWidgetItem *typeItem = new QTableWidgetItem(samples[row].type);
        typeItem->setTextAlignment(Qt::AlignCenter);
        ui->activeDiscountsTable->setItem(row, 4, typeItem);

        QTableWidgetItem *untilItem = new QTableWidgetItem(samples[row].validUntil);
        untilItem->setTextAlignment(Qt::AlignCenter);
        ui->activeDiscountsTable->setItem(row, 5, untilItem);
    }
}

// ==================== Database Methods (To be connected to real data source) ====================

QStringList ApplyDiscountWindow::getPublisherBooks() const
{
    // TODO: Connect to actual database
    // Return format: "ID|Title"
    return {
        "1|The Great Adventure",
        "2|Mystery of Shadows",
        "3|Science Today",
        "4|Poetry Collection",
        "5|History Revealed"
    };
}

double ApplyDiscountWindow::getBookPrice(int bookId) const
{
    Q_UNUSED(bookId)
    // TODO: Connect to actual database
    QMap<int, double> prices = {
        {1, 29.99},
        {2, 24.99},
        {3, 34.99},
        {4, 14.99},
        {5, 19.99}
    };
    return prices.value(bookId, 0.0);
}

QString ApplyDiscountWindow::getBookTitle(int bookId) const
{
    Q_UNUSED(bookId)
    // TODO: Connect to actual database
    return "";
}

bool ApplyDiscountWindow::saveDiscountToDatabase(int bookId, const QString &discountType,
                                                 double discountValue,
                                                 const QDateTime &startDate,
                                                 const QDateTime &endDate)
{
    Q_UNUSED(bookId)
    Q_UNUSED(discountType)
    Q_UNUSED(discountValue)
    Q_UNUSED(startDate)
    Q_UNUSED(endDate)

    // TODO: Implement actual database save
    qDebug() << "Saving discount:" << bookId << discountType << discountValue;
    return true;
}

bool ApplyDiscountWindow::removeDiscountFromDatabase(int bookId)
{
    Q_UNUSED(bookId)
    // TODO: Implement actual database removal
    qDebug() << "Removing discount from book:" << bookId;
    return true;
}

// ==================== Slot Implementations ====================

// Functionality slots
void ApplyDiscountWindow::on_bookComboBox_currentIndexChanged(int index)
{
    updateBookInfo(index);
}

void ApplyDiscountWindow::on_percentageRadio_toggled(bool checked)
{
    if (checked) {
        ui->percentageSpinBox->setEnabled(true);
        ui->percentageSpinBox->setVisible(true);
        ui->fixedAmountSpinBox->setEnabled(false);
        ui->fixedAmountSpinBox->setVisible(false);
        updateNewPricePreview();
    }
}

void ApplyDiscountWindow::on_fixedAmountRadio_toggled(bool checked)
{
    if (checked) {
        ui->fixedAmountSpinBox->setEnabled(true);
        ui->fixedAmountSpinBox->setVisible(true);
        ui->percentageSpinBox->setEnabled(false);
        ui->percentageSpinBox->setVisible(false);
        updateNewPricePreview();
    }
}

void ApplyDiscountWindow::on_percentageSpinBox_valueChanged(int value)
{
    Q_UNUSED(value)
    updateNewPricePreview();
}

void ApplyDiscountWindow::on_fixedAmountSpinBox_valueChanged(double value)
{
    Q_UNUSED(value)
    updateNewPricePreview();
}

void ApplyDiscountWindow::on_timedDiscountCheck_toggled(bool checked)
{
    ui->startDateEdit->setEnabled(checked);
    ui->endDateEdit->setEnabled(checked);
}

void ApplyDiscountWindow::on_applyDiscountButton_clicked()
{
    applyDiscountToBook();
}

void ApplyDiscountWindow::on_removeDiscountButton_clicked()
{
    removeDiscountFromBook();
}

void ApplyDiscountWindow::on_clearFormButton_clicked()
{
    clearForm();
}
