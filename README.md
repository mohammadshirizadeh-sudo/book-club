#BookClub

**A modular client-server digital library and publishing platform, built with C++ and Qt 6.**

BookClub is a desktop application that combines a digital bookstore, a personal library
manager, and a publisher portal in a single client-server ecosystem. It supports
role-based access for readers, publishers, and administrators, and includes an
integrated PDF reader, shopping cart, review system, and real-time notifications —
all backed by a custom multithreaded network protocol.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Installation & Build](#installation--build)
- [Usage](#usage)
- [Screenshots](#screenshots)
- [Roadmap](#roadmap)
- [Known Limitations](#known-limitations)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

BookClub is a multi-tier desktop application designed to demonstrate a complete
client-server product built with modern C++ and Qt. It separates concerns cleanly
across presentation (Qt Widgets), business logic (services), data access
(repositories), and networking (a socket-based server with a command factory
pattern) — making the codebase a practical reference for layered application design
in C++.

The platform supports three user roles — **Reader**, **Publisher**, and
**Administrator** — each with a dedicated set of windows and permissions, all served
by a single backend.

---

## Features

### 👤 User & Role Management
- Secure registration, login, and session tracking via `AuthService`, `UserService`,
  and `SessionManager`.
- Profile management, including info updates and password changes
  (`UserProfileWindow`, `EditInfoDialog`).
- Administrative dashboard for system oversight and platform management
  (`AdminWindow`, `AdminService`).

### 📖 Digital Library & Book Management
- Catalog browsing with dedicated search, genre, and author views
  (`SearchWindow`, `GenreBrowserWindow`, `AuthorDetailDialog`).
- Personal library and custom shelf management (`MyLibraryWindow`,
  `ShelfManagementDialog`).
- Built-in PDF reader for viewing digital books directly in the app
  (`PdfReaderWindow`).

### 🛒 E-Commerce & Publishing
- Shopping cart and checkout flow, with purchase history tracking
  (`CartWindow`, `ShoppingHistoryWindow`, `CartService`).
- Publisher portal for adding/removing books, viewing statistics, and managing
  publisher profiles (`AddBookDialog`, `DeleteBookWindow`, `BookStatisticsWindow`,
  `PublisherProfileWindow`).

### 💬 Social & Notifications
- Book reviews and ratings (`ReviewService`, `ReviewRepository`).
- Real-time in-app notifications (`NotificationWidget`, `NotificationService`).

---

## Architecture

BookClub follows a layered client-server architecture, with each tier responsible
for a distinct concern:

| Layer | Responsibility | Key Components |
|---|---|---|
| **Presentation** | Qt Widgets UI for readers, publishers, and admins | Search/library/cart windows, publisher tools, admin & server dashboards |
| **Business Logic** | Domain rules and workflows | `AuthService`, `BookService`, `CartService`, `LibraryService`, `PurchaseService`, `ReviewService`, `UserService` |
| **Data Access** | Persistence and database operations | `BookRepository`, `LibraryRepository`, `PurchaseRepository`, `ReviewRepository`, `UserRepository`, `DatabaseManager` |
| **Network** | Client-server communication | `Server`, `ClientHandler`, `CommandFactory`, `NetworkManager` |

The network tier uses a **command factory pattern**: incoming client requests are
parsed into `Request` objects, dispatched to the appropriate command handler by
`CommandFactory`, and returned to the client as `Response` objects over a
socket-based protocol. This keeps the wire protocol decoupled from business logic,
so new commands can be added without touching the networking code.

---

## Project Structure

```
book-club-main/
└── BookClubProject/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── resources.qrc
    │
    ├── appWindow/        # Top-level windows (admin, genre, publisher, user, session)
    ├── Database/         # Database initialization and management
    ├── Mutual/            # Shared dialogs and widgets (password, info, notifications)
    ├── NetworkManager/    # Client-side network manager
    ├── Publishers/        # Publisher-facing tools (add/delete books, stats, profile)
    ├── Repositories/      # Data access layer (books, library, purchases, reviews, users)
    ├── Server/            # Server core: client handling, commands, request/response
    ├── Services/          # Business logic layer
    ├── Shared/            # Shared domain models and utilities (Book, Cart, User, ...)
    └── Users/             # Reader-facing windows (search, cart, library, PDF reader)
```

> **Note:** The upstream repository currently contains two similarly named network
> directories (`Network-Manger/` and `NetworkManger/`). This is a naming
> inconsistency worth cleaning up — see [Known Limitations](#known-limitations).

---

## Requirements

| Requirement | Minimum Version |
|---|---|
| C++ compiler | C++17 (GCC, Clang, or MSVC) |
| CMake | 3.16+ |
| Qt | 6.x (Core, Gui, Widgets, Network modules) |

---

## Installation & Build

```bash
# 1. Clone the repository
git clone https://github.com/mohammadshirizadeh-sudo/book-club.git
cd book-club/BookClubProject

# 2. Configure the build
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..

# 3. Compile
cmake --build . --config Release

# 4. Run
./BookClubProject   # or the generated executable for your platform
```

Replace `/path/to/qt6` with the path to your local Qt 6 installation (for example,
`~/Qt/6.7.0/gcc_64`).

---

## Usage

1. **Start the server** — Launch the server component from the server dashboard
   or at application startup to initialize the database and begin listening for
   client connections.
2. **Sign in** — Register a new account or log in with existing credentials. Your
   role (Reader, Publisher, or Administrator) determines which features are
   available.
3. **Browse and read** — Use the search and genre browser to discover books, add
   them to your cart or library, and open them with the built-in PDF reader.
4. **Publish** *(Publisher role)* — Add or remove books and review sales
   statistics from the Publisher Portal.
5. **Administer** *(Admin role)* — Monitor system activity and manage the
   platform from the Admin Dashboard.

---

## Screenshots

> Screenshots are not yet included in this repository. Add images to a
> `docs/screenshots/` directory and reference them below.

| Window | Preview |
|---|---|
| Login & Authentication | _placeholder_ |
| User Dashboard & Book Browser | _placeholder_ |
| Integrated PDF Reader | _placeholder_ |
| Server Dashboard | _placeholder_ |

---

## Roadmap

- [ ] Token-based authentication for network socket communications
- [ ] Expanded unit test coverage across services and repositories (Google Test
      or QtTest)
- [ ] Improved UI responsiveness and multi-language localization support

---

## Known Limitations

- Network authentication currently lacks token-based security (see
  [Roadmap](#roadmap)).
- Automated test coverage is limited; most verification is manual.
- The `Network-Manger` / `NetworkManger` directory duplication in the source tree
  should be consolidated into a single, correctly named module.

---

## Contributing

Contributions are welcome. If you'd like to help:

1. Fork the repository and create a feature branch.
2. Keep changes scoped to a single concern (UI, service, repository, or network
   layer) to simplify review.
3. Follow the existing layered architecture when adding new functionality.
4. Open a pull request with a clear description of the change and its motivation.

For larger changes, consider opening an issue first to discuss the approach.

---

## License

This project is open-source. Refer to the repository's `LICENSE` file for the
full terms.


"اضافه کردن لینک آیوتی باکس به فایل ریدمی با اجازه آقای معاضد" :
https://iutbox.iut.ac.ir/index.php/s/mp9HZBtsE6MzMBi
