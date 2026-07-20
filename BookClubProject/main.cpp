#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"


#include "appWindow/genrewindow.h"
#include "appWindow/userwindow.h"
#include "appWindow/publisherwindow.h"
#include "Publishers/publisherprofilewindow.h"
// #include "../appWindow/adminwindow.h"
#include "appWindow/SessionManager.h"
#include "appWindow/userwindow.h"
#include "Users/searchwindow.h"
#include "Users/UserProfileWindow.h"
#include "Users/genrebrowserwindow.h"
#include "Users/favoritebookswindow.h"
#include "Users/mylibrarywindow.h"
#include "Users/shoppinghistorywindow.h"
#include "Server/server.h"
#include "Database/DatabaseInitializer.h"
#include "Users/cartwindow.h"


#include "Publishers/mybooks.h"

#include <QApplication>
#include <QStackedWidget>
#include <QMessageBox>
#include <QResource>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    // ===== مقداردهی دیتابیس =====
    DatabaseInitializer dbInit;
    if (!dbInit.initialize("bookclub.db")) {
        qCritical() << "❌ Failed to initialize database!";
        return -1;
    }

    Server server;
    if (!server.start(8099)) {
        qCritical() << "❌ Server failed to start!";
        return -1;
    }
    qDebug() << "✅ Server started on port 8099";

    qRegisterMetaType<Response>("Response");

    NetworkManager* networkManager = new NetworkManager();
    networkManager->connectToServer("127.0.0.1", 8099);

    // پنجره اصلی
    QStackedWidget stackedWidget;
    stackedWidget.setWindowTitle("Book Club");
    stackedWidget.resize(1500, 800);

    // صفحات
    LoginWindow* loginWindow = new LoginWindow(networkManager);
    ForgotPasswordWindow* forgotWindow = new ForgotPasswordWindow(networkManager);
    RegisterWindow* registerWindow = new RegisterWindow(networkManager);
    GenreWindow* genreWindow = new GenreWindow(networkManager);
    UserWindow* userWindow = new UserWindow(networkManager);
    PublisherWindow* publisherWindow = new PublisherWindow(networkManager);
    // AdminWindow* adminWindow = new AdminWindow();
    UserProfileWindow* profileWindow = new UserProfileWindow(networkManager);
    SearchWindow* searchWindow = new SearchWindow(networkManager);
    PublisherProfileWindow* publisherProfileWindow =  new PublisherProfileWindow(networkManager);
    MyBooks* mybooks = new MyBooks(networkManager);
    FavoriteBooksWindow* favoriteBooks = new FavoriteBooksWindow(networkManager);

    GenreBrowserWindow* genreBrowsWindow = new GenreBrowserWindow(networkManager);
    CartWindow* cartWindow = new CartWindow(networkManager);
    ShoppingHistoryWindow* shoppingWindow =new ShoppingHistoryWindow(networkManager);

    MyLibraryWindow* libraryWindow = new MyLibraryWindow(networkManager);



    // اضافه کردن صفحات
    int loginIndex = stackedWidget.addWidget(loginWindow);
    int forgotIndex = stackedWidget.addWidget(forgotWindow);
    int registerIndex = stackedWidget.addWidget(registerWindow);
    int genreIndex = stackedWidget.addWidget(genreWindow);
    int userIndex = stackedWidget.addWidget(userWindow);
    int publisherIndex = stackedWidget.addWidget(publisherWindow);
    // int adminIndex = stackedWidget.addWidget(adminWindow);
    int profileIndex = stackedWidget.addWidget(profileWindow);
    int searchIndex = stackedWidget.addWidget(searchWindow);
    int publisherProfileindex = stackedWidget.addWidget(publisherProfileWindow);
    int mybooksIndex = stackedWidget.addWidget(mybooks);
    int favBooksIndex = stackedWidget.addWidget(favoriteBooks);
    int genreBrowsWindowIndex = stackedWidget.addWidget(genreBrowsWindow);
    int cartWindowIndex = stackedWidget.addWidget(cartWindow);
    int shoppingWindowIndex  = stackedWidget.addWidget(shoppingWindow);
    int libraryWindowIndex = stackedWidget.addWidget(libraryWindow);


    //-------------------------------------------------
    // Navigation
    //-------------------------------------------------






    QObject::connect(userWindow,
                     &UserWindow::genrebrowsWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(genreBrowsWindowIndex);
                     });
    QObject::connect(userWindow,
                     &UserWindow::cartWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(cartWindowIndex);
                     });
    QObject::connect(userWindow,
                     &UserWindow::libraryWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(libraryWindowIndex);
                     });

    QObject::connect(userWindow,
                     &UserWindow::searchWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(searchIndex);
                     });




    QObject::connect(publisherWindow,
                     &PublisherWindow::myBooksWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(mybooksIndex);
                     });


    QObject::connect(userWindow,
                     &UserWindow::userProfileWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(profileIndex);
                         profileWindow->loadprof();
                     });

    QObject::connect(profileWindow,
                     &UserProfileWindow::openFavBooksWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(favBooksIndex);
                     });
    QObject::connect(profileWindow,
                     &UserProfileWindow::openShoppingHistoryDialog,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(shoppingWindowIndex);
                     });
    QObject::connect(publisherWindow,
                     &PublisherWindow::publisherProfileWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(publisherProfileindex);
                         profileWindow->loadprof();
                     });


    QObject::connect(loginWindow,
                     &LoginWindow::openForgotPasswordWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(forgotIndex);
                     });
    QObject::connect(loginWindow,
                     &LoginWindow::openUserWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(userIndex);
                         userWindow->loadFreeBooks();
                         userWindow->loadNewBooks();
                         userWindow->loadRecommendedBooks();
                         userWindow->loadBestSellers();
                     });
    QObject::connect(loginWindow,
                     &LoginWindow::openPublisherWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(publisherIndex);

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

    QObject::connect(registerWindow,
                     &RegisterWindow::openPublisherWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(publisherIndex);
                     });

    // 💡 اصلاح شده: اتصال صفحه ژانر به پنجره اصلی کاربر
    QObject::connect(genreWindow,
                     &GenreWindow::openUserWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(userIndex);

                         // 🔑 بعد از اینکه ثبت‌نام کامل شد و کاربر ژانرها را انتخاب کرد، کتاب‌ها اینجا لود می‌شوند:
                         userWindow->loadFreeBooks();
                         userWindow->loadNewBooks();
                         userWindow->loadRecommendedBooks();
                     });




    //-------------------------------------------------
    // Network Responses
    //-------------------------------------------------


    /*
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
                             userWindow->loadFreeBooks(); // لود کتاب‌ها هنگام لاگین مستقیم کاربر
                             userWindow->loadRecommendedBooks();
                             userWindow->loadNewBooks();
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


*/

    QObject::connect(networkManager,
                     &NetworkManager::errorReceived,
                     [&](const QString& message)
                     {
                         QMessageBox::critical(nullptr,
                                               "Error",
                                               message);
                     });


    QObject::connect(forgotWindow, &ForgotPasswordWindow::openUserWindow,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);
                         userWindow->loadFreeBooks();
                         userWindow->loadRecommendedBooks();
                         userWindow->loadNewBooks();
                     });
    //-------------------------------------------------

    stackedWidget.show();

    a.setStyleSheet(
        "QMessageBox QLabel { color: black; }"
        "QPushButton { color: black; }");

    int exitCode = a.exec();

    DatabaseManager::instance()->shutdown();

    return exitCode;
}