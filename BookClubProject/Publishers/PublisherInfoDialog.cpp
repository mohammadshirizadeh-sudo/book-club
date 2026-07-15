#include "PublisherInfoDialog.h"
#include "Publishers/ui_PublisherInfoDialog.h"
#include "../appWindow/SessionManager.h"
#include <QMessageBox>

PublisherInfoDialog::PublisherInfoDialog(NetworkManager* networkManager , QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PublisherInfoDialog)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &PublisherInfoDialog::handleResponse);

    // ۲. ارسال خودکار درخواست به محض ساخته شدن دیالوگ
    fetchProfileData();
}

PublisherInfoDialog::~PublisherInfoDialog()
{
    delete ui;
}


void PublisherInfoDialog::fetchProfileData()
{
    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["userId"] = userId;

    Request request(CommandType::GetProfile, params);
    qDebug() << "ℹ️ [PublisherInfoDialog] Fetching profile for userId:" << userId;

    m_networkManager->sendRequest(request);
}

void PublisherInfoDialog::handleResponse(const Response& response)
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
        ui->publisherNameLabel->setText(data["publisherName"].toString());
        ui->totalRevenueLabel->setText(data["totalRevenue"].toString());
        ui->joinedAtLabel->setText(data["joinedAt"].toString());

        qDebug() << "✅ [PublisherInfoDialog] UI successfully updated with profile data.";
    }
}