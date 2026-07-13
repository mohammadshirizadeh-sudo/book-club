#include "editinfodialog.h"
#include "ui_editinfodialog.h"

#include "../appWindow/SessionManager.h"
#include "../Server/Request.h"
#include <QMessageBox>
#include <QDebug>

EditInfoDialog::EditInfoDialog(NetworkManager* networkManager ,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditInfoDialog)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &EditInfoDialog::handleResponse);
}

EditInfoDialog::~EditInfoDialog()
{
    delete ui;
}


void EditInfoDialog::on_buttonBox_accepted()
{
    // ۱. دریافت مقادیر از فیلدهای متنی (حذف فاصله‌های اضافی ابتدایی و انتهایی)
    QString email = ui->changeEmaiLineEdit->text().trimmed();
    QString fullName = ui->changeFullnameLineEdit->text().trimmed();
    QString userName = ui->changeUsernameLineEdit->text().trimmed();

    // ۲. اعتبارسنحی اولیه در کلاینت (خالی نبودن فیلدها)
    if (email.isEmpty() || fullName.isEmpty() || userName.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً تمامی فیلدها را پر کنید.", QMessageBox::Ok);
        return;
    }

    // ۳. دریافت آیدی کاربر فعلی
    int userId = SessionManager::instance()->getUserId();

    // ۴. بسته‌بندی پارامترها (دقیقاً هماهنگ با نام کلیدهای سرور)
    QVariantMap params;
    params["userId"] = userId;
    params["email"] = email;
    params["fullName"] = fullName;
    params["userName"] = userName; // دقت کنید: N بزرگ است!

    // ۵. ساخت و ارسال درخواست
    Request request(CommandType::UpdateProfile, params);
    qDebug() << "📝 [Client] Sending UpdateProfile request for userId:" << userId;

    m_networkManager->sendRequest(request);
}

\

    void EditInfoDialog::handleResponse(const Response& response)
{
    // مدیریت پاسخ ویرایش پروفایل
    if (response.getCommandType() == CommandType::UpdateProfile) {

        if (!response.isSuccess()) {
            // نمایش خطای سرور (مثلاً تکراری بودن ایمیل یا نام کاربری)
            QMessageBox::critical(this, "خطا در به‌روزرسانی", response.getMessage());
            return;
        }

        // در صورت موفقیت‌آمیز بودن
        QMessageBox::information(this, "موفقیت", response.getMessage());

        // بستن دیالوگ با وضعیت تایید شده (Accept)
        // این کار باعث می‌شود پنجره قبلی متوجه تغییرات شود و اطلاعات را رفرش کند
        this->accept();
    }
}
