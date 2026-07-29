#include "SignWindow/loginwindow.h"
#include "SignWindow/registerwindow.h"
#include "SignWindow/forgotpasswordwindow.h"

#include "appWindow/genrewindow.h"
#include "appWindow/userwindow.h"
#include "appWindow/publisherwindow.h"
#include "appWindow/adminwindow.h"
#include "appWindow/SessionManager.h"
#include "appWindow/userwindow.h"

#include "Publishers/addbookdialog.h"
#include "Publishers/applydiscountwindow.h"
#include "Publishers/deactivatebookwindow.h"
#include "Publishers/editbookswindow.h"
#include "Publishers/publisherprofilewindow.h"

#include "Users/AuthorDetailDialog.h"
#include "Users/BookDetailDialog.h"
#include "Users/bookreviewdialog.h"
#include "Users/cartwindow.h"
#include "Users/favoritebookswindow.h"
#include "Users/genrebrowserwindow.h"
#include "Users/mylibrarywindow.h"
#include "Users/pdfreaderwindow.h"
#include "Users/searchwindow.h"
#include "Users/shelfmanagementdialog.h"
#include "Users/shoppinghistorywindow.h"
#include "Users/UserDetailDialog.h"
#include "Users/UserProfileWindow.h"

#include "Mutual/changepassworddialog.h"
#include "Mutual/editinfodialog.h"
#include "Mutual/infodialog.h"
#include "Mutual/notificationwidget.h"
#include "appWindow/adminwindow.h"
#include "Publishers/publishedbookswindow.h"
#include "Users/groupreadingwindow.h"

#include "Server/server.h"

#include "Database/DatabaseInitializer.h"
#include "Database/DatabaseManager.h"

#include <QApplication>
#include <QStackedWidget>
#include <QMessageBox>
#include <QResource>
#include <QScreen>
#include <QGuiApplication>

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

    // پنجره اصلی. میخوام دقیقا وسط قرار بگیره همه صفهات
    QStackedWidget stackedWidget;
    stackedWidget.setWindowTitle("Book Club");
    stackedWidget.resize(1500, 800);

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
    FavoriteBooksWindow* favoriteBooks = new FavoriteBooksWindow(networkManager);

    GenreBrowserWindow* genreBrowsWindow = new GenreBrowserWindow(networkManager);
    CartWindow* cartWindow = new CartWindow(networkManager);
    ShoppingHistoryWindow* shoppingWindow =new ShoppingHistoryWindow(networkManager);

    MyLibraryWindow* libraryWindow = new MyLibraryWindow(networkManager);

    ShelfManagementDialog* shelfWindow = new ShelfManagementDialog(networkManager);
    ApplyDiscountWindow* applydiscountWindow = new ApplyDiscountWindow(networkManager);
    EditBooksWindow* editWindow = new EditBooksWindow(networkManager);
    DeactivateBookWindow* deactivateBook = new DeactivateBookWindow(networkManager);
    NotificationWidget* notificatoinWindow = new NotificationWidget(networkManager);
    AdminWindow* adminWindow = new AdminWindow(networkManager);
    PublishedBooksWindow* publishedWindow = new PublishedBooksWindow(networkManager);
    GroupReadingWindow* groupreadingWindow = new GroupReadingWindow(networkManager);







    int loginIndex = stackedWidget.addWidget(loginWindow);
    int forgotIndex = stackedWidget.addWidget(forgotWindow);
    int registerIndex = stackedWidget.addWidget(registerWindow);
    int genreIndex = stackedWidget.addWidget(genreWindow);
    int userIndex = stackedWidget.addWidget(userWindow);
    int publisherIndex = stackedWidget.addWidget(publisherWindow);
    //int adminIndex = stackedWidget.addWidget(adminWindow);
    int profileIndex = stackedWidget.addWidget(profileWindow);
    int searchIndex = stackedWidget.addWidget(searchWindow);
    int publisherProfileindex = stackedWidget.addWidget(publisherProfileWindow);
    int favBooksIndex = stackedWidget.addWidget(favoriteBooks);
    int genreBrowsWindowIndex = stackedWidget.addWidget(genreBrowsWindow);
    int cartWindowIndex = stackedWidget.addWidget(cartWindow);
    int shoppingWindowIndex  = stackedWidget.addWidget(shoppingWindow);
    int libraryWindowIndex = stackedWidget.addWidget(libraryWindow);
    int shelfWindowIndex = stackedWidget.addWidget(shelfWindow);
    int applyDiscountIndex = stackedWidget.addWidget(applydiscountWindow);
    int editWindowIndex = stackedWidget.addWidget(editWindow);
    int deactivateWindowIndex = stackedWidget.addWidget(deactivateBook);
    int notificationIndex = stackedWidget.addWidget(notificatoinWindow);
    int adminWindowIndex = stackedWidget.addWidget(adminWindow);
    int publishedBookIndex = stackedWidget.addWidget(publishedWindow);
    // Bug fix: GroupReadingWindow was constructed but never added to the
    // stack, so it had no index and could never actually be navigated to.
    int groupReadingIndex = stackedWidget.addWidget(groupreadingWindow);




    QObject::connect(loginWindow,
                     &LoginWindow::openAdminWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(adminWindowIndex);
                         adminWindow->setAdminInfo();

                         adminWindow->initializeFromServer();
                     });

    QObject::connect(userWindow,
                     &UserWindow::genrebrowsWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(genreBrowsWindowIndex);
                     });
    // The old no-arg connection from UserWindow::groubReadingWindow to a
    // stack-index switch was removed: BookDetailDialog now opens its own
    // dedicated GroupReadingWindow directly from its "Group Reading" button
    // (see BookDetailDialog::openGroupReading), so the signal-chain through
    // UserWindow is no longer used. The QStackedWidget-owned
    // `groupreadingWindow` instance and its index below are kept as a
    // landing page in case a future entry point (e.g. a sidebar button)
    // wants to navigate to it directly.
    QObject::connect(userWindow,
                     &UserWindow::cartWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(cartWindowIndex);
                     });


    QObject::connect(userWindow,
                     &UserWindow::notificationWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(notificationIndex);
                     });
    QObject::connect(userWindow,
                     &UserWindow::libraryWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(libraryWindowIndex);
                     });
    QObject::connect(userWindow,
                     &UserWindow::shelfWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(shelfWindowIndex);
                     });


    QObject::connect(userWindow,
                     &UserWindow::searchWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(searchIndex);
                     });

    QObject::connect(publisherWindow,
                     &PublisherWindow::applydiscountWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(applyDiscountIndex);
                     });

    QObject::connect(publisherWindow,
                     &PublisherWindow::notificationWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(notificationIndex);
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


    QObject::connect(publisherWindow,
                     &PublisherWindow::editWindow,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(editWindowIndex);

                     });
    QObject::connect(publisherWindow,
                     &PublisherWindow::deactivateBook,
                     [&]()
                     {
                         stackedWidget.setCurrentIndex(deactivateWindowIndex);

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
    QObject::connect(adminWindow, &AdminWindow::editWindow,
                     [&]() {
                         stackedWidget.setCurrentIndex(editWindowIndex);

                     });
    QObject::connect(publisherWindow, &PublisherWindow::publishedBookWindow,
                     [&]() {
                         stackedWidget.setCurrentIndex(publishedBookIndex);

                     });






    //backpush buttoms
    QObject::connect(cartWindow, &CartWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });

    QObject::connect(libraryWindow, &MyLibraryWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(searchWindow, &SearchWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(shelfWindow, &ShelfManagementDialog::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(profileWindow, &UserProfileWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(shoppingWindow, &ShoppingHistoryWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(profileIndex);

                     });
    QObject::connect(favoriteBooks, &FavoriteBooksWindow::backButtonClicked,
                     [&]() {
                         stackedWidget.setCurrentIndex(profileIndex);

                     });
    QObject::connect(genreBrowsWindow, &GenreBrowserWindow::userWindow,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(genreBrowsWindow, &GenreBrowserWindow::userWindow,
                     [&]() {
                         stackedWidget.setCurrentIndex(userIndex);

                     });
    QObject::connect(publisherProfileWindow, &PublisherProfileWindow::backPushButton,
                     [&]() {
                         stackedWidget.setCurrentIndex(publisherIndex);

                     });
    QObject::connect(publishedWindow, &PublishedBooksWindow::backRequested,
                     [&]() {
                         stackedWidget.setCurrentIndex(publisherIndex);

                     });
    QObject::connect(editWindow, &EditBooksWindow::backPushButton,
                     [&]() {
                         stackedWidget.setCurrentIndex(publisherIndex);

                     });
    QObject::connect(applydiscountWindow, &ApplyDiscountWindow::backPushButton,
                     [&]() {
                         stackedWidget.setCurrentIndex(publisherIndex);

                     });
    QObject::connect(deactivateBook, &DeactivateBookWindow::backPushButton,
                     [&]() {
                         stackedWidget.setCurrentIndex(publisherIndex);

                     });



    QObject::connect(adminWindow, &AdminWindow::signOutRequested,
                     [&]() {
                         SessionManager::instance()->clear();
                         stackedWidget.setCurrentIndex(loginIndex);
                     });



    QObject::connect(
        publisherWindow,
        &PublisherWindow::signOutRequested,
        [&]()
        {
            SessionManager::instance()->clear();

            stackedWidget.setCurrentIndex(loginIndex);
        });

    QObject::connect(
        userWindow,
        &UserWindow::signOutRequested,
        [&]()
        {
            SessionManager::instance()->clear();

            stackedWidget.setCurrentIndex(loginIndex);
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