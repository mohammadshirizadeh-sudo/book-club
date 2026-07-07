#include "registerwindow.h"
#include "signWindow/ui_registerwindow.h"

#include "../Shared/EmailValidator.h"
#include "../Shared/PasswordValidator.h"
#include "../Repositories/UserRepository.h"

#include <QMessageBox>

RegisterWindow::RegisterWindow(NetworkManager* networkManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
    , m_networkManager(networkManager)
{
    ui->setupUi(this);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::on_userSignupPushButton_clicked()
{
    // Check if passwords match
    QString password = ui->passwordNewLineEdit->text();
    QString confirmPassword = ui->passwordNewLineEdit_2->text();

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Password Mismatch", "The passwords do not match. Please try again.");
        return;
    }

    // Check if email is valid
    QString email = ui->emailNewLineEdit->text().trimmed();

    ValidationResult result = EmailValidator::isValidForLogin(email);

    if (!result.isValid) {
        QMessageBox::warning(this, "Invalid Email", result.errorMessage);
        return;
    }

    // 3. ساخت درخواست ثبت‌نام و ارسال به سرور
    QVariantMap params;
    params["username"] = ui->userNewLineEdit->text().trimmed();
    params["email"] = email;
    params["password"] = password;
    params["fullName"] = ui->fullnameNewLineEdit->text().trimmed();
    params["role"] = "User";

    Request request(CommandType::Register, params);
    // ارسال درخواست به سرور
    m_networkManager->sendRequest("Register", params);

    emit openGenreWindow();
}

void RegisterWindow::on_publisherSignupPushButton_clicked()
{
    // Check if passwords match
    QString password = ui->passwordNewLineEdit->text();
    QString confirmPassword = ui->passwordNewLineEdit_2->text();

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Password Mismatch", "The passwords do not match. Please try again.");
        return;
    }

    // Check if email is valid
    QString email = ui->emailNewLineEdit->text().trimmed();

    ValidationResult result = EmailValidator::isValidForLogin(email);

    if (!result.isValid) {
        QMessageBox::warning(this, "Invalid Email", result.errorMessage);
        return;
    }

    QVariantMap params;
    params["username"] = ui->userNewLineEdit->text().trimmed();
    params["email"] = email;
    params["password"] = password;
    params["fullName"] = ui->fullnameNewLineEdit->text().trimmed();
    params["role"] = "Publisher";

    Request request(CommandType::Register, params);
    m_networkManager->sendRequest("Register", params);

    emit openPublisherWindow();
}

void RegisterWindow::on_backToSigninPushButton_clicked()
{
    emit openLoginWindow();
}