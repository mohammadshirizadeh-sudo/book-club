#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"

#include "appWindow/genrewindow.h"
#include "appWindow/userwindow.h"
#include "appWindow/publisherwindow.h"
#include "appWindow/adminwindow.h"
#include "appWindow/SessionManager.h"

#include <QApplication>
#include <QStackedWidget>
#include <QMessageBox>
#include <QResource>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // برای Signal/Slotهایی که Response ارسال می‌کنند
    qRegisterMetaType<Response>("Response");

    // ایجاد NetworkManager
    NetworkManager* networkManager = new NetworkManager();
    networkManager->connectToServer("127.0.0.1", 8080);

    // پنجره اصلی
    QStackedWidget stackedWidget;
    stackedWidget.setWindowTitle("Book Club");
    stackedWidget.resize(1500, 800);

    // صفحات
    LoginWindow* loginWindow = new LoginWindow(networkManager);
    ForgotPasswordWindow* forgotWindow = new ForgotPasswordWindow();
    RegisterWindow* registerWindow = new RegisterWindow(networkManager);
    GenreWindow* genreWindow = new GenreWindow();
    UserWindow* userWindow = new UserWindow();
    PublisherWindow* publisherWindow = new PublisherWindow();
    AdminWindow* adminWindow = new AdminWindow();

    // اضافه کردن صفحات
    int loginIndex = stackedWidget.addWidget(loginWindow);
    int forgotIndex = stackedWidget.addWidget(forgotWindow);
    int registerIndex = stackedWidget.addWidget(registerWindow);
    int genreIndex = stackedWidget.addWidget(genreWindow);
    int userIndex = stackedWidget.addWidget(userWindow);
    int publisherIndex = stackedWidget.addWidget(publisherWindow);
    int adminIndex = stackedWidget.addWidget(adminWindow);

    //-------------------------------------------------
    // Navigation
    //-------------------------------------------------

    QObject::connect(loginWindow,
                     &LoginWindow::openForgotPasswordWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(forgotIndex);
                     });

    QObject::connect(forgotWindow,
                     &ForgotPasswordWindow::openLoginWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(loginIndex);
                     });

    QObject::connect(loginWindow,
                     &LoginWindow::openRegisterWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(registerIndex);
                     });

    QObject::connect(registerWindow,
                     &RegisterWindow::openLoginWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(loginIndex);
                     });

    QObject::connect(registerWindow,
                     &RegisterWindow::openGenreWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(genreIndex);
                     });

    // از main دوم
    QObject::connect(registerWindow,
                     &RegisterWindow::openPublisherWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(publisherIndex);
                     });

    QObject::connect(genreWindow,
                     &GenreWindow::openUserWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(userIndex);
                     });

    //-------------------------------------------------
    // Network Responses
    //-------------------------------------------------

    QObject::connect(networkManager,
                     &NetworkManager::successReceived,
                     [&](const QVariantMap& data)
                     {
                         if (!data.contains("userId"))
                             return;

                         QString role = data["role"].toString();
                         int userId = data["userId"].toInt();
                         QString username = data["username"].toString();

                         SessionManager::instance()->setCurrentUser(
                             userId,
                             username,
                             role);

                         if (role == "User")
                         {
                             stackedWidget.setCurrentIndex(userIndex);
                         }
                         else if (role == "Publisher")
                         {
                             stackedWidget.setCurrentIndex(publisherIndex);
                         }
                         else if (role == "Admin")
                         {
                             stackedWidget.setCurrentIndex(adminIndex);
                         }
                     });

    QObject::connect(networkManager,
                     &NetworkManager::errorReceived,
                     [&](const QString& message)
                     {
                         QMessageBox::critical(nullptr,
                                               "خطا",
                                               message);
                     });

    //-------------------------------------------------

    stackedWidget.show();

    a.setStyleSheet(
        "QMessageBox QLabel { color: white; }"
        "QPushButton { color: white; }");

    return a.exec();
}