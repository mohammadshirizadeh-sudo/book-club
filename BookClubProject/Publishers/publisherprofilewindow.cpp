#include "publisherprofilewindow.h"
// #include "PublisherInfoDialog.h"
#include "../Mutual/infodialog.h"
#include "Publishers/ui_publisherprofilewindow.h"
#include "../Mutual/changepassworddialog.h"
#include "../Mutual/editinfodialog.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include <QMessageBox>

PublisherProfileWindow::PublisherProfileWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PublisherProfileWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &PublisherProfileWindow::handleResponse);
}


PublisherProfileWindow::~PublisherProfileWindow()
{
    delete ui;
}




void PublisherProfileWindow::on_changePubPassPushButton_clicked()
{
    qDebug()<<"hi cris";
    ChangePasswordDialog dialog(m_networkManager , this);
    dialog.exec();
}


void PublisherProfileWindow::on_publisherInfoPushButton_clicked()
{
    InfoDialog dialog(m_networkManager , this);
    dialog.exec();
}



void PublisherProfileWindow::loadprof()
{
    // درخواست کتاب‌های رایگان

    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["userId"] = userId;

    qDebug() << "📚 [Client] Sending GetProdile request to server...";
    Request request(CommandType::GetProfile, params);
    m_networkManager->sendRequest(request);
}


void PublisherProfileWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::GetProfile) {
        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "خطا در دریافت اطلاعات: " + response.getMessage());
            return;
        }

        // دیتای دریافتی را مستقیماً روی UI همین دیالوگ می‌نشانیم
        QVariantMap data = response.getData();

        ui->publishernameLabel->setText(data["username"].toString());
        ui->totalRevenueLabel->setText(data["totalRevenue"].toString());

        qDebug() << "✅ [InfoDialog] UI successfully updated with profile data.";
    }
}

void PublisherProfileWindow::on_editPubInfoPushButton_clicked()
{
    EditInfoDialog* dialog = new EditInfoDialog(m_networkManager, this);

    // ✅ اتصال به سیگنال accepted دیالوگ
    connect(dialog, &QDialog::accepted, this, [this]() {
        qDebug() << "📢 Dialog accepted! Reloading user profile...";
        loadprof();
    });

    dialog->exec();
}

