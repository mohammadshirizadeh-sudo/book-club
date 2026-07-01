#include "appWindow/mainwindow.h"
#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"
#include <QResource>


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow loginWindow;
    ForgotPasswordWindow forgotWindow;
    RegisterWindow registerWindow;
    MainWindow mainWindow;

    loginWindow.show();

    // Login -> Forgot Password
    QObject::connect(&loginWindow,
                     &LoginWindow::openForgotPasswordWindow,
                     [&]()
                     {
                         loginWindow.close();
                         forgotWindow.show();
                     });

    // Forgot Password -> Main
    QObject::connect(&forgotWindow,
                     &ForgotPasswordWindow::openMainWindow,
                     [&]()
                     {
                         forgotWindow.close();
                         mainWindow.show();
                     });

    // Login -> Register
    QObject::connect(&loginWindow,
                     &LoginWindow::openRegisterWindow,
                     [&]()
                     {
                         loginWindow.close();
                         registerWindow.show();
                     });

    // Register -> Main
    QObject::connect(&registerWindow,
                     &RegisterWindow::openMainWindow,
                     [&]()
                     {
                         registerWindow.close();
                         mainWindow.show();
                     });

    // Register -> Login
    QObject::connect(&registerWindow,
                     &RegisterWindow::openLoginWindow,
                     [&]()
                     {
                         registerWindow.close();
                         loginWindow.show();
                     });

    // Login -> Main
    QObject::connect(&loginWindow,
                     &LoginWindow::openMainWindow,
                     [&]()
                     {
                         loginWindow.close();
                         mainWindow.show();
                     });

    return a.exec();
}