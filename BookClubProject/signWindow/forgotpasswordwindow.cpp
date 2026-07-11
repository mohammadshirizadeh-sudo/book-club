#include "forgotpasswordwindow.h"
#include "../Server/Request.h"
#include <QMessageBox>
#include <QClipboard>
#include "../appWindow/SessionManager.h"

#include "signWindow/ui_forgotpasswordwindow.h"

ForgotPasswordWindow::ForgotPasswordWindow(NetworkManager* networkManager,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ForgotPasswordWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &ForgotPasswordWindow::handleResponse);

}


ForgotPasswordWindow::~ForgotPasswordWindow()
{
    delete ui;
}

void ForgotPasswordWindow::on_tokenPushButton_clicked()
{
    // ۱. دریافت ایمیل از فیلد متنی و حذف فاصله‌های اضافی
    QString email = ui->forgotEmailLineEdit->text().trimmed();

    // ۲. بررسی خالی نبودن فیلد ایمیل در سمت کلاینت
    if (email.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً آدرس ایمیل خود را وارد کنید.", QMessageBox::Ok);
        return;
    }

    // ۳. آماده‌سازی پارامترها دقیقاً با همان کلیدی که سرور منتظر آن است ("email")
    QVariantMap params;
    params["email"] = email;

    // ۴. ساخت شیء درخواست با کامند مربوطه
    Request request(CommandType::RequestPasswordReset, params);

    qDebug() << "🔑 [Client] Sending RequestPasswordReset request for email:" << email;

    // ۵. ارسال درخواست به سرور
    m_networkManager->sendRequest(request);
}




void ForgotPasswordWindow::handleResponse(const Response& response)
{
    // مدیریت پاسخ درخواست ریست پسورد
    if (response.getCommandType() == CommandType::RequestPasswordReset) {

        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", response.getMessage());
            return;
        }

        // اگر سرور با موفقیت پاسخ داد، داده‌ها را استخراج می‌کنیم
        QVariantMap data = response.getData();
        QString token = data["token"].toString();
        QString email = data["email"].toString();
        QString expiry = data["expiry"].toString();

        // 📋 شبیه‌سازی: کپی کردن خودکار توکن در کلیپ‌بورد سیستم برای تست راحت‌تر
        QApplication::clipboard()->setText(token);

        // 💬 ساختن پیام برای نشان دادن به کاربر همراه با توکن واقعی صادر شده از سرور
        QString simulationMessage = QString(
                                        "درخواست بازیابی رمز عبور با موفقیت انجام شد.\n\n"
                                        "⚠️ [حالت شبیه‌سازی توسعه]\n"
                                        "توکن صادر شده برای شما:\n"
                                        "➡️  %1\n\n"
                                        "این توکن به صورت خودکار در حافظه (Clipboard) کپی شد. می‌توانید در مرحله بعد آن را Paste کنید.\n"
                                        "زمان انقضا: %2"
                                        ).arg(token).arg(expiry);

        QMessageBox::information(this, "موفقیت (حالت توسعه)", simulationMessage);

        // اینجا می‌توانید کاربر را به بخش یا اسلاید بعدی (وارد کردن توکن و رمز جدید) هدایت کنید
        // مثلا اگر از QStackedWidget استفاده می‌کنید:
        // ui->stackedWidget->setCurrentIndex(صفحه_ورود_توکن);
    }



    if (response.getCommandType() == CommandType::ResetPasswordWithToken) {

        if (!response.isSuccess()) {
            // نمایش خطای سرور (مثلاً منقضی شدن توکن یا اشتباه بودن آن)
            QMessageBox::critical(this, "خطا در بازیابی", response.getMessage());
            return;
        }

        // اگر تغییر رمز موفقیت‌آمیز بود
        QMessageBox::information(this, "موفقیت", "رمز عبور شما با موفقیت تغییر یافت.\nاکنون می‌توانید وارد حساب خود شوید.");


        QVariantMap data = response.getData();
        int userId = data.value("userId").toInt();
        QString username = data.value("username").toString();
        QString role = data.value("role").toString();

        // ✅ ذخیره در Session
        SessionManager::instance()->setCurrentUser(userId, username, role);
        // پاکسازی فیلدها برای امنیت بیشتر
        ui->lineEdit->clear();
        ui->newPassLineEdit->clear();
        ui->newPassLineEdit_2->clear();
        emit openUserWindow();
    }
}
void ForgotPasswordWindow::on_remmemberPushButton_clicked()
{
    // ۱. دریافت مقادیر از فیلدهای متنی و حذف فضاهای خالی اضافه
    QString token = ui->lineEdit->text().trimmed();
    QString newPassword = ui->newPassLineEdit->text(); // رمز عبور را trimmed نکنید چون فاصله می‌تواند جزوی از رمز باشد

    // ۲. اعتبارسنحی اولیه در سمت کلاینت برای جلوگیری از ارسال درخواست بیهوده
    if (token.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً توکن ارسال شده را وارد کنید.", QMessageBox::Ok);
        return;
    }

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "خطا", "لطفاً رمز عبور جدید خود را وارد کنید.", QMessageBox::Ok);
        return;
    }

    // کلاینت می‌تواند بررسی‌های بیشتری مثل حداقل طول رمز (مثلاً ۶ کاراکتر) را نیز اینجا انجام دهد:
    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "خطا", "رمز عبور جدید باید حداقل ۶ کاراکتر باشد.", QMessageBox::Ok);
        return;
    }

    // ۳. بسته‌بندی پارامترها با کلیدهای دقیقاً یکسان با سرور ("token" و "newPassword")
    QVariantMap params;
    params["token"] = token;
    params["newPassword"] = newPassword;

    // ۴. ساخت شیء درخواست با کامند مربوطه
    Request request(CommandType::ResetPasswordWithToken, params);

    qDebug() << "🔄 [Client] Sending ResetPasswordWithToken request...";

    // ۵. ارسال درخواست به سرور
    m_networkManager->sendRequest(request);
}

