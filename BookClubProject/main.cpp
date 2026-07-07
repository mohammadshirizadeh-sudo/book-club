#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"
#include "appWindow/genrewindow.h"
#include "appWindow/userwindow.h"
#include "appWindow/publisherwindow.h"
#include "appWindow/adminwindow.h"
#include "appWindow/SessionManager.h"

#include <QResource>
#include <QApplication>
#include <QStackedWidget>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. ایجاد NetworkManager و اتصال به سرور
    NetworkManager* networkManager = new NetworkManager();
    networkManager->connectToServer("127.0.0.1", 8080);

    // 2. ایجاد QStackedWidget به عنوان پنجره اصلی
    QStackedWidget stackedWidget;
    stackedWidget.setWindowTitle("Book Club");
    stackedWidget.resize(1500, 800); // سایز پیش‌فرض پروژه

    // 3. ایجاد تمام ویندوزها (با پاس دادن NetworkManager)
    LoginWindow* loginWindow = new LoginWindow(networkManager);
    ForgotPasswordWindow* forgotWindow = new ForgotPasswordWindow();
    RegisterWindow* registerWindow = new RegisterWindow(networkManager);
    GenreWindow* genreWindow = new GenreWindow();
    UserWindow* userWindow = new UserWindow();
    PublisherWindow* publisherWindow = new PublisherWindow();
    AdminWindow* adminWindow = new AdminWindow();

    // 4. اضافه کردن ویندوزها به QStackedWidget
    int loginIndex = stackedWidget.addWidget(loginWindow);
    int forgotIndex = stackedWidget.addWidget(forgotWindow);
    int registerIndex = stackedWidget.addWidget(registerWindow);
    int genreIndex = stackedWidget.addWidget(genreWindow);
    int userIndex = stackedWidget.addWidget(userWindow);
    int publisherIndex = stackedWidget.addWidget(publisherWindow);
    int adminIndex = stackedWidget.addWidget(adminWindow);

    // =============================================
    // ====== اتصالات (Signals & Slots) =============
    // =============================================

    // --- Login -> Forgot Password ---
    QObject::connect(loginWindow, &LoginWindow::openForgotPasswordWindow, [&]() {
        stackedWidget.setCurrentIndex(forgotIndex);
    });

    // --- Forgot Password -> Back to Login ---
    // (چون در فایل قبلی این سیگنال نبود، باید در ForgotPasswordWindow.h تعریف کنید: void openLoginWindow();)
    QObject::connect(forgotWindow, &ForgotPasswordWindow::openLoginWindow, [&]() {
        stackedWidget.setCurrentIndex(loginIndex);
    });

    // --- Login -> Register ---
    QObject::connect(loginWindow, &LoginWindow::openRegisterWindow, [&]() {
        stackedWidget.setCurrentIndex(registerIndex);
    });

    // --- Register -> Login ---
    QObject::connect(registerWindow, &RegisterWindow::openLoginWindow, [&]() {
        stackedWidget.setCurrentIndex(loginIndex);
    });

    // =============================================
    // ====== مدیریت پاسخ‌های شبکه =================
    // =============================================

    // --- لاگین موفق ---
    QObject::connect(networkManager, &NetworkManager::successReceived, [&](const QVariantMap& data) {
        // اگر پاسخ مربوط به لاگین بود (بررسی وجود userId)
        if (data.contains("userId")) {
            QString role = data.value("role").toString();
            int userId = data.value("userId").toInt();
            QString username = data.value("username").toString();

            // ذخیره در SessionManager
            SessionManager::instance()->setCurrentUser(userId, username, role);

            // هدایت بر اساس نقش
            if (role == "User") {
                // اگر کاربر قبلاً ژانر انتخاب نکرده، به Genre برو، وگرنه به UserWindow
                // فعلاً فرض می‌کنیم همیشه به UserWindow می‌رود
                stackedWidget.setCurrentIndex(userIndex);
            }
            else if (role == "Publisher") {
                stackedWidget.setCurrentIndex(publisherIndex);
            }
            else if (role == "Admin") {
                stackedWidget.setCurrentIndex(adminIndex);
            }
        }
    });

    // --- خطای شبکه ---
    QObject::connect(networkManager, &NetworkManager::errorReceived, [&](const QString& message) {
        QMessageBox::critical(nullptr, "خطا", message);
    });

    // =============================================
    // ====== مدیریت ثبت‌نام و انتخاب ژانر =========
    // =============================================

    // --- Register -> درخواست ثبت‌نام موفق شد (این‌جا می‌توانید هندل کنید) ---
    // (فعلاً طبق منطق قبلی، پس از ثبت‌نام به Genre می‌رود)
    QObject::connect(registerWindow, &RegisterWindow::openGenreWindow, [&]() {
        stackedWidget.setCurrentIndex(genreIndex);
    });

    // --- Genre -> User (پس از انتخاب ۱-۳ ژانر) ---
    QObject::connect(genreWindow, &GenreWindow::openUserWindow, [&]() {
        stackedWidget.setCurrentIndex(userIndex);
    });

    // --- نمایش پنجره اصلی ---
    stackedWidget.show();

    a.setStyleSheet(
        "QMessageBox QLabel { color: white; }"
        "QPushButton { color: white; }"
        );

    return a.exec();
}