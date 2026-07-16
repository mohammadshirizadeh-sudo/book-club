#include "UserProfileWindow.h"
#include "Users/ui_UserProfileWindow.h"

#include "../appWindow/SessionManager.h"
#include "../Network-Manger/NetworkManager.h"
#include "../Server/Request.h"
#include "../Server/Response.h"
#include "../Mutual/infodialog.h"
#include "../Mutual/changepassworddialog.h"
#include "../Mutual/editinfodialog.h"
#include <QMessageBox>

UserProfileWindow::UserProfileWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserProfileWindow)
        , m_networkManager(networkManager)
{
    ui->setupUi(this);

    connect(m_networkManager, &NetworkManager::responseReceived,
            this, &UserProfileWindow::handleResponse);
}

UserProfileWindow::~UserProfileWindow()
{
    delete ui;
}

void UserProfileWindow::on_favGenresPushButton_clicked()
{
    emit openGenreWindow();
}


void UserProfileWindow::on_UserInfPushButton_clicked()
{
    EditInfoDialog* dialog = new EditInfoDialog(m_networkManager, this);

    // ✅ اتصال به سیگنال accepted دیالوگ
    connect(dialog, &QDialog::accepted, this, [this]() {
        qDebug() << "📢 Dialog accepted! Reloading user profile...";
        loadprof();
    });

    dialog->exec();
}


void UserProfileWindow::on_editUserInfPushButton_clicked()
{
    EditInfoDialog dialog(m_networkManager , this);
    dialog.exec();

}


void UserProfileWindow::on_changePassPushButton_clicked()
{
    ChangePasswordDialog dialog(m_networkManager , this);
    dialog.exec();
}


void UserProfileWindow::on_shoppingHistoryPushButton_clicked()
{
    emit openShoppingHistoryDialog();
}


void UserProfileWindow::on_favBooksPushButton_clicked()
{
    emit openFavBooksDialog();
}


void UserProfileWindow::loadprof()
{
    // درخواست کتاب‌های رایگان

    int userId = SessionManager::instance()->getUserId();

    QVariantMap params;
    params["userId"] = userId;

    qDebug() << "📚 [Client] Sending GetProdile request to server...";
    Request request(CommandType::GetProfile, params);
    m_networkManager->sendRequest(request);
}


void UserProfileWindow::handleResponse(const Response& response)
{
    if (response.getCommandType() == CommandType::GetProfile) {
        if (!response.isSuccess()) {
            QMessageBox::critical(this, "خطا", "خطا در دریافت اطلاعات: " + response.getMessage());
            return;
        }

        // دیتای دریافتی را مستقیماً روی UI همین دیالوگ می‌نشانیم
        QVariantMap data = response.getData();

        ui->usernameLabel->setText(data["username"].toString());
        ui->purchasedBooksLabel->setText(data["purchaseCount"].toString());

        qDebug() << "✅ [InfoDialog] UI successfully updated with profile data.";
    }
}

