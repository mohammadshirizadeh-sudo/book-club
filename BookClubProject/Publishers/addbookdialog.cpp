#include "addbookdialog.h"
#include "Publishers/ui_addbookdialog.h"

#include "../Server/Request.h"
#include "../Server/Response.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QMimeDatabase>
#include <QFileInfo>

AddBookDialog::AddBookDialog(NetworkManager* networkManager, int publisherId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddBookDialog)
    , m_networkManager(networkManager)
    , m_publisherId(publisherId)
{
    ui->setupUi(this);

    // Connect network manager response signal
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &AddBookDialog::onResponseReceived);

    // Initial form validation
    validateForm();
}

AddBookDialog::~AddBookDialog()
{
    delete ui;
}

void AddBookDialog::clearForm()
{
    ui->bookNameLineEdit->clear();
    ui->authorNameLineEdit->clear();
    ui->genreComboBox->setCurrentIndex(0);
    ui->priceSpinBox->setValue(9.99);
    ui->discountSpinBox->setValue(0);
    ui->descriptionTextEdit->clear();

    m_coverImageData.clear();
    m_pdfData.clear();
    m_coverFileName.clear();
    m_pdfFileName.clear();

    ui->coverPreviewLabel->setText("Cover Preview\n\n(JPG, PNG, WEBP)\nMax: 5MB");
    ui->coverPreviewLabel->setPixmap(QPixmap());
    ui->coverFileSizeLabel->setText("");
    ui->coverStatusLabel->setText("");

    ui->pdfFileNameLabel->setText("No file selected\n(Max: 50MB)");
    ui->pdfFileSizeLabel->setText("");
    ui->pdfStatusLabel->setText("");

    ui->charCountLabel->setText("0 chars");

    validateForm();
}

bool AddBookDialog::isFormValid() const
{
    // Check required fields
    if (ui->bookNameLineEdit->text().trimmed().isEmpty()) return false;
    if (ui->authorNameLineEdit->text().trimmed().isEmpty()) return false;
    if (ui->genreComboBox->currentIndex() <= 0) return false; // "Select Genre..." is index 0
    if (m_coverImageData.isEmpty()) return false;
    if (m_pdfData.isEmpty()) return false;

    return true;
}

void AddBookDialog::validateForm()
{
    bool valid = isFormValid();
    ui->submitButton->setEnabled(valid);

    if (valid) {
        ui->submitButton->setToolTip("Submit book to server");
    } else {
        ui->submitButton->setToolTip("Please fill all required fields first");
    }
}

void AddBookDialog::updateCharCount()
{
    int count = ui->descriptionTextEdit->toPlainText().length();
    ui->charCountLabel->setText(QString("%1 chars").arg(count));
}

bool AddBookDialog::validateCoverImage(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    // Check file size (max 5MB = 5 * 1024 * 1024 bytes)
    qint64 maxSize = 5 * 1024 * 1024;
    if (fileInfo.size() > maxSize) {
        QMessageBox::warning(this, "File Too Large",
                             QString("Cover image is too large.\nMaximum size: 5 MB\nCurrent size: %1 MB")
                                 .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 2));
        return false;
    }

    // Check MIME type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(fileInfo);
    QStringList allowedMimes;
    allowedMimes << "image/jpeg" << "image/png" << "image/webp";

    if (!allowedMimes.contains(mimeType.name())) {
        QMessageBox::warning(this, "Invalid Format",
                             "Unsupported image format.\nPlease use JPG, PNG, or WEBP.");
        return false;
    }

    return true;
}

bool AddBookDialog::validatePdfFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    // Check file size (max 50MB)
    qint64 maxSize = 50 * 1024 * 1024;
    if (fileInfo.size() > maxSize) {
        QMessageBox::warning(this, "File Too Large",
                             QString("PDF file is too large.\nMaximum size: 50 MB\nCurrent size: %1 MB")
                                 .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 2));
        return false;
    }

    // Check MIME type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(fileInfo);

    if (mimeType.name() != "application/pdf") {
        QMessageBox::warning(this, "Invalid Format",
                             "Selected file is not a valid PDF.");
        return false;
    }

    return true;
}

void AddBookDialog::updateCoverPreview(const QPixmap &pixmap)
{
    if (!pixmap.isNull()) {
        QPixmap scaled = pixmap.scaled(
            ui->coverPreviewLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );
        ui->coverPreviewLabel->setPixmap(scaled);
        ui->coverPreviewLabel->setText("");

        ui->coverStatusLabel->setStyleSheet(
            "QLabel { background: transparent; font: 700 9pt \"Script MT Bold\"; "
            "font-size: 14px; color: rgb(40, 167, 69); }"
            );
        ui->coverStatusLabel->setText("✓ Cover image loaded successfully");
    } else {
        ui->coverPreviewLabel->setPixmap(QPixmap());
        ui->coverPreviewLabel->setText("Failed to load image");

        ui->coverStatusLabel->setStyleSheet(
            "QLabel { background: transparent; font: 700 9pt \"Script MT Bold\"; "
            "font-size: 14px; color: rgb(220, 53, 69); }"
            );
        ui->coverStatusLabel->setText("✗ Failed to load cover image");
    }
}

void AddBookDialog::updatePdfDisplay(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        ui->pdfFileNameLabel->setText(
            QString("📄 %1\nSize: %2 KB")
                .arg(fileInfo.fileName())
                .arg(fileInfo.size() / 1024)
            );

        ui->pdfStatusLabel->setStyleSheet(
            "QLabel { background: transparent; font: 700 9pt \"Script MT Bold\"; "
            "font-size: 14px; color: rgb(40, 167, 69); }"
            );
        ui->pdfStatusLabel->setText("✓ PDF file selected successfully");
    } else {
        ui->pdfFileNameLabel->setText("No file selected\n(Max: 50MB)");

        ui->pdfStatusLabel->setStyleSheet(
            "QLabel { background: transparent; font: 700 9pt \"Script MT Bold\"; "
            "font-size: 14px; color: rgb(220, 53, 69); }"
            );
        ui->pdfStatusLabel->setText("✗ No PDF file selected");
    }
}

// ===== Slot Implementations =====

void AddBookDialog::on_browseCoverButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Cover Image",
        QString(),
        "Images (*.jpg *.jpeg *.png *.webp);;All Files (*)"
        );

    if (!filePath.isEmpty()) {
        if (validateCoverImage(filePath)) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                m_coverImageData = file.readAll();
                m_coverFileName = QFileInfo(filePath).fileName();

                // Update preview
                QPixmap pixmap;
                if (pixmap.loadFromData(m_coverImageData)) {
                    updateCoverPreview(pixmap);

                    // Update file size label
                    double sizeMB = m_coverImageData.size() / (1024.0 * 1024.0);
                    ui->coverFileSizeLabel->setText(
                        QString("%1 MB").arg(sizeMB, 0, 'f', 2)
                        );
                } else {
                    updateCoverPreview(QPixmap());
                }

                file.close();
                validateForm();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Could not open cover image file:\n" + file.errorString());
            }
        }
    }
}

void AddBookDialog::on_browsePdfButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select PDF File",
        QString(),
        "PDF Files (*.pdf);;All Files (*)"
        );

    if (!filePath.isEmpty()) {
        if (validatePdfFile(filePath)) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                m_pdfData = file.readAll();
                m_pdfFileName = QFileInfo(filePath).fileName();

                // Update display
                updatePdfDisplay(filePath);

                // Update file size label
                double sizeMB = m_pdfData.size() / (1024.0 * 1024.0);
                ui->pdfFileSizeLabel->setText(
                    QString("%1 MB").arg(sizeMB, 0, 'f', 2)
                    );

                file.close();
                validateForm();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Could not open PDF file:\n" + file.errorString());
            }
        }
    }
}

void AddBookDialog::on_clearButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Clear",
        "Are you sure you want to clear all fields?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        clearForm();
    }
}
/*
void AddBookDialog::on_submitButton_clicked()
{
    if (!isFormValid()) {
        QMessageBox::warning(this, "Incomplete Form",
                             "Please fill in all required fields before submitting.");
        return;
    }

    // Confirm submission
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Submission",
        "Are you sure you want to submit this book?\n\nTitle: " + ui->bookNameLineEdit->text() +
            "\nAuthor: " + ui->authorNameLineEdit->text(),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QVariantMap params;
        params["publisherId"] = m_publisherId;
        params["title"] = ui->bookNameLineEdit->text().trimmed();
        params["author"] = ui->authorNameLineEdit->text().trimmed();
        params["genre"] = ui->genreComboBox->currentText();
        params["price"] = ui->priceSpinBox->value();
        params["discount"] = ui->discountSpinBox->value();
        params["description"] = ui->descriptionTextEdit->toPlainText();
        params["coverImage"] = m_coverImageData.toBase64();
        params["coverFileName"] = m_coverFileName;
        params["pdfData"] = m_pdfData.toBase64();
        params["pdfFileName"] = m_pdfFileName;

        Request request(CommandType::AddBook, params);
        m_networkManager->sendRequest(request);

        // Disable submit button while processing
        ui->submitButton->setEnabled(false);
        ui->submitButton->setText("Submitting...");
    }
}
*/


void AddBookDialog::on_submitButton_clicked()
{
    if (!isFormValid()) {
        QMessageBox::warning(this, "Incomplete Form",
                             "Please fill in all required fields before submitting.");
        return;
    }

    // Confirm submission
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Submission",
        "Are you sure you want to submit this book?\n\nTitle: " + ui->bookNameLineEdit->text() +
            "\nAuthor: " + ui->authorNameLineEdit->text(),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QVariantMap params;
        params["publisherId"] = m_publisherId;
        params["title"] = ui->bookNameLineEdit->text().trimmed();
        params["author"] = ui->authorNameLineEdit->text().trimmed();
        params["genre"] = ui->genreComboBox->currentText();
        params["price"] = ui->priceSpinBox->value();
        params["discount"] = ui->discountSpinBox->value();
        params["description"] = ui->descriptionTextEdit->toPlainText();

        // 🟢 تغییر اول: تغییر کلید "coverImage" به "coverData" برای هماهنگی کامل با پروتکل سرور و ساختار UserWindow
        // 🟢 تغییر دوم: استفاده از QString::fromUtf8 برای تبدیل آرایه باینری Base64 به متن استاندارد جهت ارسال بدون مشکل در پروتکل JSON
        params["coverData"] = QString::fromUtf8(m_coverImageData.toBase64());
        params["coverFileName"] = m_coverFileName;

        // 🟢 تغییر سوم: تبدیل داده باینری PDF به متن Base64 با ساختار QString برای جلوگیری از خراب شدن فایل در طول مسیر شبکه
        params["pdfData"] = QString::fromUtf8(m_pdfData.toBase64());
        params["pdfFileName"] = m_pdfFileName;

        Request request(CommandType::AddBook, params);
        m_networkManager->sendRequest(request);

        // Disable submit button while processing
        ui->submitButton->setEnabled(false);
        ui->submitButton->setText("Submitting...");
    }
}

void AddBookDialog::onResponseReceived(const Response& response)
{
    switch (response.getCommandType()) {
    case CommandType::AddBook:
        if (response.isSuccess()) {
            QMessageBox::information(this, "Success",
                                     "Book added successfully!\nIt will be reviewed by admin shortly.");

            emit bookAddedSuccessfully();
            accept(); // Close dialog on success
        } else {
            QMessageBox::warning(this, "Submission Failed",
                                 "Failed to add book:\n" + response.getMessage());

            ui->submitButton->setEnabled(true);
            ui->submitButton->setText("✓ Submit");
        }
        break;

    default:
        break;
    }
}

void AddBookDialog::on_pdfStatusLabel_linkActivated(const QString &link)
{

}

