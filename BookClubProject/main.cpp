#include "mainwindow.h"
#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"
#include <QResource>
#include <QFile>
#include <QDebug>
#include <QDir>





#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    LoginWindow w1;
    w1.show();

    RegisterWindow w2;
    w2.show();

    ForgotPasswordWindow w3;
    w3.show();



    //MainWindow w;
    //w.show();
    return QApplication::exec();
}
