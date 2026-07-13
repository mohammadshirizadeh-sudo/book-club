#include "changepassworddialog.h"
#include "ui_changepassworddialog.h"

#include "../Server/Request.h"
#include "../appWindow/SessionManager.h"

#include <QMessageBox>
#include <QDebug>

ChangePasswordDialog::ChangePasswordDialog(NetworkManager* networkManager ,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangePasswordDialog)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ChangePasswordDialog::handleResponse);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}



void ChangePasswordDialog::handleResponse(const Response& response)
{
    // مدیریت پاسخ تغییر رمز عبور
    if (response.getCommandType() == CommandType::ChangePassword) {

        if (!response.isSuccess()) {
            // نمایش خطای سرور (مثلاً رمز عبور قدیمی اشتباه است)
            QMessageBox::critical(this, "خطا", response.getMessage());
            return;
        }

        // در صورت موفقیت آمیز بودن
        QMessageBox::information(this, "موفقیت", "رمز عبور شما با موفقیت تغییر یافت.");

        // پاکسازی فیلدها و بستن دیالوگ
        ui->oldPassLineEdit->clear();
        ui->newPassLineEdit->clear();

        this->accept(); // بستن دیالوگ با وضعیت پذیرفته شده
    }
}
void ChangePasswordDialog::on_buttonBox_accepted()
{
    QString oldPassword = ui->oldPassLineEdit->text();
    QString newPassword = ui->newPassLineEdit->text();
    QString confirmPassword = ui->newPassLineEdit_2->text();
    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "خطا", "رمز عبور جدید با تکرار آن مطابقت ندارد.");
        return;
    }

    // ۲. اعتبارسنحی اولیه در سمت کلاینت
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً تمام فیلدها را پر کنید.", QMessageBox::Ok);
        return;
    }

    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "خطا", "رمز عبور جدید باید حداقل ۶ کاراکتر باشد.", QMessageBox::Ok);
        return;
    }

    // ۳. دریافت آیدی کاربر فعلی که لاگین کرده است
    int userId = SessionManager::instance()->getUserId();

    // ۴. بسته‌بندی پارامترها با کلیدهای دقیقاً یکسان با متغیرهای سرور
    QVariantMap params;
    params["userId"] = userId;
    params["oldPassword"] = oldPassword;
    params["newPassword"] = newPassword;

    // ۵. ساخت و ارسال درخواست به سرور
    Request request(CommandType::ChangePassword, params);
    qDebug() << "🔄 [Client] Sending ChangePassword request for userId:" << userId;

    m_networkManager->sendRequest(request);
}

