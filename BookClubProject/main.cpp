#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"
#include "appWindow/genrewindow.h"
#include "appWindow/userwindow.h"
#include "appWindow/publisherwindow.h"
#include <QResource>


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NetworkManager* networkManager = new NetworkManager();
    networkManager->connectToServer("127.0.0.1", 8080);


    LoginWindow loginWindow(networkManager);
    ForgotPasswordWindow forgotWindow;
    RegisterWindow registerWindow;
    GenreWindow genreWindow;
    UserWindow userWindow;
    PublisherWindow publisherWindow;

    loginWindow.show();

    // Login -> Forgot Password
    QObject::connect(&loginWindow,
                    &LoginWindow::openForgotPasswordWindow,
                    [&]()
                    {
                        loginWindow.close();
                        forgotWindow.show();
                    });
/*
    // Forgot Password -> Main
    QObject::connect(&forgotWindow,
                    &ForgotPasswordWindow::openMainWindow,
                    [&]()
                    {
                        forgotWindow.close();
                        mainWindow.show();
                    });
*/
    // Login -> Register
    QObject::connect(&loginWindow,
                    &LoginWindow::openRegisterWindow,
                    [&]()
                    {
                        loginWindow.close();
                        registerWindow.show();
                    });

    // Register -> Genre
    QObject::connect(&registerWindow,
                    &RegisterWindow::openGenreWindow,
                    [&]()
                    {
                        registerWindow.close();
                        genreWindow.show();
                    });

    // Register -> Publisher
    QObject::connect(&registerWindow,
                    &RegisterWindow::openPublisherWindow,
                    [&]()
                    {
                        registerWindow.close();
                        publisherWindow.show();
                    });

    // Register -> Login
    QObject::connect(&registerWindow,
                    &RegisterWindow::openLoginWindow,
                    [&]()
                    {
                        registerWindow.close();
                        loginWindow.show();
                    });
/*
    // Login -> Main
    QObject::connect(&loginWindow,
                    &LoginWindow::openMainWindow,
                    [&]()
                    {
                        loginWindow.close();
                        mainWindow.show();
                    });
*/

    // Genre -> User
    QObject::connect(&genreWindow,
                    &GenreWindow::openUserWindow,
                    [&]()
                    {
                        genreWindow.close();
                        userWindow.show();
                    });

    a.setStyleSheet(
        "QMessageBox QLabel { color: white; }"
        "QPushButton { color: white; }"
        );

    return a.exec();
}