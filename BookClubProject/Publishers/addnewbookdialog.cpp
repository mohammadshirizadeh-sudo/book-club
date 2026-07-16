#include "addnewbookdialog.h"
#include "Publishers/ui_addnewbookdialog.h"
#include "../appWindow/SessionManager.h"
#include <QFileDialog>
#include <QUuid>
#include <QMessageBox>




AddNewBookDialog::AddNewBookDialog(NetworkManager* networkManager,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddNewBookDialog)
    ,m_networkManager(networkManager)
{
    ui->setupUi(this);
    ui->bookCoverPreviewLabel->setAlignment(Qt::AlignCenter);


    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &AddNewBookDialog::handleResponse);
}

AddNewBookDialog::~AddNewBookDialog()
{
    delete ui;
}

void AddNewBookDialog::on_addPictureButton_clicked()
{


    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Select Book Cover"),
        QDir::homePath(),
        tr("Images (*.png *.jpg *.jpeg)")
        );

    if (filePath.isEmpty()) {
        return; // کاربر منصرف شده و دیالوگ را بسته است
    }

    // ۲. ساخت مسیر پوشه ذخیره‌سازی محلی (پوشه‌ای به اسم covers در کنار فایل اجرایی برنامه)
    QString targetDir = QCoreApplication::applicationDirPath() + "/covers";
    QDir dir;
    if (!dir.exists(targetDir)) {
        dir.mkpath(targetDir); // اگر پوشه نبود آن را می‌سازد
    }

    // ۳. تولید یک نام یونیک و تصادفی برای عکس تا عکس‌های هم‌نام روی هم بازنویسی نشوند
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix(); // استخراج پسوند فایل (مثلا png)
    QString uniqueName = QUuid::createUuid().toString(QUuid::WithoutBraces) + "." + extension;
    QString destinationPath = targetDir + "/" + uniqueName;

    // ۴. کپی کردن عکس انتخابی به پوشه مقصد پروژه
    if (QFile::copy(filePath, destinationPath)) {
        // ذخیره مسیر نسبی برای دیتابیس (مثال: covers/a1b2c3d4.png)
        m_savedCoverPath = "covers/" + uniqueName;

        // ۵. نمایش پیش‌نمایش تصویر در QLabel با افکت Smooth
        QPixmap pixmap(destinationPath);
        if (!pixmap.isNull()) {
            ui->bookCoverPreviewLabel->setPixmap(
                pixmap.scaled(ui->bookCoverPreviewLabel->size(),
                              Qt::KeepAspectRatio,
                              Qt::SmoothTransformation)
                );
        }
    } else {
        QMessageBox::critical(this, "Error", "Could not save the image. Please try again.");
    }
}





void AddNewBookDialog::on_buttonBox_accepted()
{

    if (ui->lable10->text().trimmed().isEmpty() ||
        ui->lable12->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in the Title and Author fields!");
        return;
    }


    /*
    // ۲. غیرفعال کردن دکمه‌ها برای جلوگیری از اسپم کلیک
    ui->okButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);
*/

    // ۳. آماده‌سازی پارامترها
    QVariantMap params;
    params["publisherId"] = SessionManager::instance()->getUserId();
    params["title"] = ui->titleLineEdit->text();
    params["author"] = ui->authorLineEdit->text();
    params["genre"] = ui->genreComboBox->currentText();
    params["description"] = ui->descriptionTextEdit->toPlainText();
    params["price"] = ui->priceDoubleSpinBox->value();
    params["coverPath"] = m_savedCoverPath;

    // اگر عکس انتخاب شده بود، بایت‌های آن را به Base64 تبدیل کرده و می‌فرستیم
    if (!m_imageBytes.isEmpty()) {
        params["coverData"] = QString(m_imageBytes.toBase64());
    }

    // ۴. ارسال درخواست به سرور
    Request request(CommandType::AddBook, params);
    m_networkManager->sendRequest(request);

}

void AddNewBookDialog::handleResponse(const Response& response)
{
    // فقط پاسخ‌های مربوط به AddBook را بررسی کن
    if (response.getCommandType() != CommandType::AddBook) {
        return;
    }


    /*
    ui->okButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);
*/

    if (response.isSuccess()) {
        QMessageBox::information(this, "Success", "Book added successfully!");
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Failed to add book: " + response.getMessage());
    }
}

