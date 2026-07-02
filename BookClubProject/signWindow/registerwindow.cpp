#include "registerwindow.h"
#include "ui_registerwindow.h"

#include "../Shared/EmailValidator.h"
#include "../Shared/PasswordValidator.h"
#include "../Repositories/UserRepository.h"

#include <QMessageBox>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
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

    emit openPublisherWindow();
}

void RegisterWindow::on_backToSigninPushButton_clicked()
{
    emit openLoginWindow();
}