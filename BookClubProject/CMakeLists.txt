cmake_minimum_required(VERSION 3.19)
project(BookClubProject LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(BookClubProject
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Shared/User.h Shared/User.cpp
    Shared/PasswordHelper.h
    Shared/PasswordValidator.h
    Shared/EmailValidator.h
    Shared/Genre.h
    Shared/Book.h Shared/Book.cpp
    Shared/admin.h Shared/admin.cpp
    Shared/Review.h Shared/Review.cpp
    Shared/Purchase.h Shared/Purchase.cpp
    Shared/Notification.h Shared/Notification.cpp
    Shared/Cart.h Shared/Cart.cpp
    Shared/CartItem.h Shared/CartItem.cpp
    Shared/Publisher.h Shared/Publisher.cpp
    Shared/UserFactory.h
    Services/AuthService.h Services/AuthService.cpp
    Services/PublisherService.h Services/PublisherService.cpp
    Shared/ValidationResult.h
    Shared/PasswordValidator.cpp
    Shared/EmailValidator.cpp
    Services/BookService.h Services/BookService.cpp
    Services/CartService.h Services/CartService.cpp
    Services/NotificationService.h Services/NotificationService.cpp
    Services/PurchaseService.h Services/PurchaseService.cpp
    Services/ReviewService.h Services/ReviewService.cpp
    Services/UserService.h Services/UserService.cpp
    Shared/Shelf.h Shared/Shelf.cpp
    Repositories/BookRepository.h Repositories/BookRepository.cpp
    Repositories/LibraryRepository.h Repositories/LibraryRepository.cpp
    Repositories/PurchaseRepository.h Repositories/PurchaseRepository.cpp
    Shared/Library.h Shared/Library.cpp
    Repositories/ReviewRepository.h Repositories/ReviewRepository.cpp
    Repositories/UserRepository.h Repositories/UserRepository.cpp
    Server/Server.h Server/Server.cpp
    Server/ClientHandler.h Server/ClientHandler.cpp
    Server/RequestParser.h Server/RequestParser.cpp
    Shared/ValidationResult.cpp

)

target_link_libraries(BookClubProject
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS BookClubProject
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET BookClubProject
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
