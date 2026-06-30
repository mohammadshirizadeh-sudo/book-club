#include "appWindow/mainwindow.h"
#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"
#include <QResource>


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
/*
    LoginWindow login;
    RegisterWindow registerWin;
    login.show();
    QObject::connect(&login, &LoginWindow::openRegisterWindow,
                     [&]() {
                         login.close();        // بستن صفحه اول
                         registerWin.show();   // باز کردن صفحه دوم
                     });
*/  //اینو واسه وصل کردن صفحه ها نوشتم ولی جواب نمیده، مثل 2 روش دیگه ای که امتحان کردم

    //LoginWindow w1;
    //w1.show();

    //RegisterWindow w2;
    //w2.show();

    //ForgotPasswordWindow w3;
    //w3.show();

    //MainWindow w;
    //w.show();

    return QApplication::exec();
}
