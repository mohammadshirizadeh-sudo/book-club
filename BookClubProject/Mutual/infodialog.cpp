#include "infodialog.h"
#include "ui_infodialog.h"
#include "../appWindow/SessionManager.h"
#include <QMessageBox>

InfoDialog::InfoDialog(NetworkManager* networkManager , QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InfoDialog)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &InfoDialog::handleResponse);

    // ۲. ارسال خودکار درخواست به محض ساخته شدن دیالوگ
    fetchProfileData();
}

InfoDialog::~InfoDialog()
{
    delete ui;
}


void InfoDialog::fetchProfileData()
{
    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetProfile, params);
    qDebug() << "ℹ️ [InfoDialog] Fetching profile for userId:" << userId;

    m_networkManager->sendRequest(request);
}

void InfoDialog::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::GetProfile) {
        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "خطا در دریافت اطلاعات: " + response.getMessage());
            return;
        }

        // دیتای دریافتی را مستقیماً روی UI همین دیالوگ می‌نشانیم
        QVariantMap data = response.getData();

        ui->usernameLabel->setText(data["username"].toString());
        ui->emailLabel->setText(data["email"].toString());
        ui->fullnameLabel->setText(data["fullName"].toString());
        ui->roleLabel->setText(data["role"].toString());
        ui->statusLabel->setText(data["status"].toString());

        QStringList favoriteGenres = data["favoriteGenres"].toStringList();
        ui->favGenresLabel->setText(favoriteGenres.isEmpty() ? "بدون ژانر محبوب" : favoriteGenres.join("، "));

        qDebug() << "✅ [InfoDialog] UI successfully updated with profile data.";
    }
}