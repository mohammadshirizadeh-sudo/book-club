// userrepository.cpp
#include "UserRepository.h"
#include "../Shared/Genre.h"

#include <QDebug>

UserRepository::UserRepository(QObject* parent) :QObject(parent){
    loadAllFromDatabase();
}

UserRepository::~UserRepository() {
    // Clean up all users
    clearCache();
}



/*
bool UserRepository::addUser(User* user) {
    if (!user) return false;

    int id = user->getId();

    if (usersById.contains(id)) {
        qWarning() << "User with ID" << id << "already exists!";
        return false;
    }


    if (isUsernameTaken(user->getUsername())) {
        qWarning() << "Username" << user->getUsername() << "is already taken!";
        return false;
    }


    if (isEmailTaken(user->getEmail())) {
        qWarning() << "Email" << user->getEmail() << "is already taken!";
        return false;
    }


    addToCache(user);

    // 2. Save to SQLite
    if (!saveToDatabase(user)) {
        // Rollback: remove from cache if DB fails
        removeFromCache(id);
        return false;
    }


    return true;
}
*/


bool UserRepository::addUser(User* user) {
    if (!user) return false;


    DatabaseManager* db = DatabaseManager::instance();
    db->transaction();

    if (!saveToDatabase(user)) {
        db->rollback();
        return false;
    }

    bool success = true;

    if (Publisher* publisher = dynamic_cast<Publisher*>(user)) {
        success = savePublisherInfo(publisher);
        if (!success) {
            qWarning() << "Failed to save Publisher info for user" << user->getId();
        }
    } else if (Admin* admin = dynamic_cast<Admin*>(user)) {
        success = saveAdminInfo(admin);
        if (!success) {
            qWarning() << "Failed to save Admin info for user" << user->getId();
        }
    }

    if (success) {
        db->commit();

        {
            QMutexLocker locker(&m_mutex);
            addToCache(user);
        }
        qDebug() << "✅ User added successfully:" << user->getId();
        return true;
    } else {
        db->rollback();
        qWarning() << "❌ Failed to add user - rolling back";
        return false;
    }
}

User* UserRepository::findById(int id) const {
    QMutexLocker locker(&m_mutex);
    return usersById.value(id, nullptr);
}

User* UserRepository::findByUsername(const QString& username) const {
    QMutexLocker locker(&m_mutex);
    return usersByUsername.value(username, nullptr);
}

User* UserRepository::findByEmail(const QString& email) const {
    QMutexLocker locker(&m_mutex);
    return usersByEmail.value(email, nullptr);
}

QVector<User*> UserRepository::getAllUsers() const {
    QMutexLocker locker(&m_mutex);
    return usersById.values().toVector();
}


/*
bool UserRepository::updateUser(User* user, const QString& oldUsername, const QString& oldEmail) {
    if (!user) return false;

    if (!saveToDatabase(user)) {
        return false;
    }

    if (oldUsername != user->getUsername()) {
        usersByUsername.remove(oldUsername);
    }
    if (oldEmail != user->getEmail()) {
        usersByEmail.remove(oldEmail);
    }

    // 3. اضافه کردن کلیدهای جدید
    usersById[user->getId()] = user;
    usersByUsername[user->getUsername()] = user;
    usersByEmail[user->getEmail()] = user;

    return true;
}*/

bool UserRepository::updateUser(User* user, const QString& oldUsername, const QString& oldEmail) {
    if (!user) return false;

    DatabaseManager* db = DatabaseManager::instance();
    db->transaction();

    if (!saveToDatabase(user)) {
        db->rollback();
        return false;
    }

    bool success = true;

    if (Publisher* publisher = dynamic_cast<Publisher*>(user)) {
        success = savePublisherInfo(publisher);
        if (!success) {
            qWarning() << "Failed to update Publisher info for user" << user->getId();
        }
    } else if (Admin* admin = dynamic_cast<Admin*>(user)) {
        success = saveAdminInfo(admin);
        if (!success) {
            qWarning() << "Failed to update Admin info for user" << user->getId();
        }
    }

    if (success) {
        db->commit();
        {

            QMutexLocker locker(&m_mutex);

            if (oldUsername != user->getUsername()) {
                usersByUsername.remove(oldUsername);
            }
            if (oldEmail != user->getEmail()) {
                usersByEmail.remove(oldEmail);
            }

            usersById[user->getId()] = user;
            usersByUsername[user->getUsername()] = user;
            usersByEmail[user->getEmail()] = user;
        }

        qDebug() << "✅ User updated successfully:" << user->getId();
        return true;
    } else {
        db->rollback();
        qWarning() << "❌ Failed to update user - rolling back";
        return false;
    }
}




bool UserRepository::deleteUser(int userId) {
    QMutexLocker locker(&m_mutex);
    User* user = usersById.value(userId, nullptr);
    if (!user) {
        qWarning() << "User with ID" << userId << "not found!";
        return false;
    }


    if (!deleteFromDatabase(userId)) {
        qWarning() << "Failed to delete user from database!";
        return false;
    }

    removeFromCache(userId);
    delete user;

    return true;
}

bool UserRepository::isUsernameTaken(const QString& username) const {
    QMutexLocker locker(&m_mutex);
    return usersByUsername.contains(username);
}

bool UserRepository::isEmailTaken(const QString& email) const {
    QMutexLocker locker(&m_mutex);
    return usersByEmail.contains(email);
}

void UserRepository::resetNextId() {
    QMutexLocker locker(&m_mutex);
    int maxId = 1000;
    for (int id : usersById.keys()) {
        if (id > maxId) {
            maxId = id;
        }
    }
    nextId = maxId + 1;
    qDebug() << "nextId reset to:" << nextId;
}

/*
bool UserRepository::loadAllFromDatabase()
{
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT id, username, email, password_hash, salt, full_name,
               role, status, favorite_genres, created_at, updated_at,
               last_login, reset_token, reset_token_expiry
        FROM user
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load users:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        // Create User from SQLite data
        User* user = new User(
            sqlQuery.value("id").toInt(),
            sqlQuery.value("full_name").toString(),
            sqlQuery.value("username").toString(),
            sqlQuery.value("email").toString(),
            stringToRole(sqlQuery.value("role").toString()),
            stringToStatus(sqlQuery.value("status").toString()),
            QDateTime::fromString(sqlQuery.value("created_at").toString(), Qt::ISODate),
            QDateTime::fromString(sqlQuery.value("last_login").toString(), Qt::ISODate),
            sqlQuery.value("password_hash").toString(),
            stringToGenres(sqlQuery.value("favorite_genres").toString()),
            QDateTime::fromString(sqlQuery.value("updated_at").toString(), Qt::ISODate),
            sqlQuery.value("salt").toString()
            );



        // Set reset token if exists
        QString resetToken = sqlQuery.value("reset_token").toString();
        if (!resetToken.isEmpty()) {
            user->setPasswordResetToken(resetToken);
            QDateTime expiry = QDateTime::fromString(
                sqlQuery.value("reset_token_expiry").toString(), Qt::ISODate
                );
            user->setResetTokenExpiry(expiry);
        }

        addToCache(user);
        count++;
    }

    resetNextId();
    qDebug() << "✅ Loaded" << count << "users from SQLite";
    return true;
}
*/


bool UserRepository::loadAllFromDatabase() {
    QMutexLocker locker(&m_mutex);
    clearCache();

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        SELECT id, username, email, password_hash, salt, full_name,
               role, status, favorite_genres, created_at, updated_at,
               last_login, reset_token, reset_token_expiry
        FROM user
    )";

    QSqlQuery sqlQuery = db->executeSelect(query);

    if (sqlQuery.lastError().isValid()) {
        qCritical() << "Failed to load users:" << sqlQuery.lastError().text();
        return false;
    }

    int count = 0;
    while (sqlQuery.next()) {
        int id = sqlQuery.value("id").toInt();
        QString fullName = sqlQuery.value("full_name").toString();
        QString username = sqlQuery.value("username").toString();
        QString email = sqlQuery.value("email").toString();
        UserRole role = stringToRole(sqlQuery.value("role").toString());
        AccountStatus status = stringToStatus(sqlQuery.value("status").toString());
        QDateTime createdAt = QDateTime::fromString(
            sqlQuery.value("created_at").toString(), Qt::ISODate
            );
        QDateTime lastLogin = QDateTime::fromString(
            sqlQuery.value("last_login").toString(), Qt::ISODate
            );
        QString passwordHash = sqlQuery.value("password_hash").toString();
        QVector<Genre> genres = stringToGenres(
            sqlQuery.value("favorite_genres").toString()
            );
        QDateTime updatedAt = QDateTime::fromString(
            sqlQuery.value("updated_at").toString(), Qt::ISODate
            );
        QString salt = sqlQuery.value("salt").toString();

        User* user = nullptr;
        switch (role) {
        case UserRole::Publisher: {
            // ساخت Publisher واقعی
            Publisher* publisher = new Publisher(
                id, fullName, username, email,
                role, status,
                createdAt, lastLogin,
                passwordHash, genres, updatedAt,
                salt
                );

            loadPublisherInfo(publisher);

            user = publisher;
            break;
        }

        case UserRole::Admin: {
            // ساخت Admin واقعی
            Admin* admin = new Admin(
                id, fullName, username, email,
                role, status,
                createdAt, lastLogin,
                passwordHash, genres, updatedAt,
                salt
                );

            loadAdminInfo(admin);

            user = admin;
            break;
        }

        default: {
            // User عادی
            user = new User(
                id, fullName, username, email,
                role, status,
                createdAt, lastLogin,
                passwordHash, genres, updatedAt,
                salt
                );
            break;
        }
        }

        QString resetToken = sqlQuery.value("reset_token").toString();
        if (!resetToken.isEmpty()) {
            user->setPasswordResetToken(resetToken);
            QDateTime expiry = QDateTime::fromString(
                sqlQuery.value("reset_token_expiry").toString(), Qt::ISODate
                );
            user->setResetTokenExpiry(expiry);
        }

        addToCache(user);
        count++;
    }

    resetNextId();
    qDebug() << "✅ Loaded" << count << "users from SQLite";
    return true;
}


bool UserRepository::saveToDatabase(User* user)
{
    if (!user) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "Database is not open!";
        return false;
    }

    QString query = R"(
        INSERT OR REPLACE INTO user (
            id, username, email, password_hash, salt, full_name,
            role, status, favorite_genres, created_at, updated_at,
            last_login, reset_token, reset_token_expiry
        ) VALUES (
            :id, :username, :email, :password_hash, :salt, :full_name,
            :role, :status, :favorite_genres, :created_at, :updated_at,
            :last_login, :reset_token, :reset_token_expiry
        )
    )";

    QVariantMap params;
    params["id"] = user->getId();
    params["username"] = user->getUsername();
    params["email"] = user->getEmail();
    params["password_hash"] = user->getPasswordHash();
    params["salt"] = user->getSalt();
    params["full_name"] = user->getFullname();
    params["role"] = user->getRoleString();
    params["status"] = statusToString(user->getStatus());
    params["favorite_genres"] = genresToString(user->getFavouriteGenre());
    params["created_at"] = user->getCreatedAt().toString(Qt::ISODate);
    params["updated_at"] = user->getUpdatedAt().toString(Qt::ISODate);
    params["last_login"] = user->getLastLogin().toString(Qt::ISODate);
    params["reset_token"] = user->getPasswordResetToken();
    params["reset_token_expiry"] = user->getResetTokenExpiry().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}


void UserRepository::loadPublisherInfo(Publisher* publisher) {
    if (!publisher) {
        qWarning() << "loadPublisherInfo: publisher is null!";
        return;
    }

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "loadPublisherInfo: Database is not open!";
        return;
    }

    QString query = "SELECT * FROM publisher_info WHERE user_id = :user_id";

    QVariantMap params;
    params["user_id"] = publisher->getId();

    QSqlQuery result = DatabaseManager::instance()->executeSelect(query, params);


    if (result.lastError().isValid()) {
        qWarning() << "loadPublisherInfo: Query failed for user_id"
                   << publisher->getId() << ":" << result.lastError().text();
        return;
    }

    if (result.next()) {
        publisher->setPublisherName(result.value("publisher_name").toString());
        publisher->setTotalRevenue(result.value("total_revenue").toDouble());
        publisher->setJoinedAt(
            QDateTime::fromString(result.value("joined_at").toString(), Qt::ISODate)
            );
        qDebug() << "✅ Loaded publisher info for user" << publisher->getId();
    }else {
        qDebug() << "No publisher info found for user" << publisher->getId();
    }
}

bool UserRepository::savePublisherInfo(Publisher* publisher) {
    if (!publisher) return false;

    QString query = R"(
        INSERT OR REPLACE INTO publisher_info (
            user_id, publisher_name, total_revenue, joined_at
        ) VALUES (
            :user_id, :publisher_name, :total_revenue, :joined_at
        )
    )";

    QVariantMap params;
    params["user_id"] = publisher->getId();
    params["publisher_name"] = publisher->getPublisherName();
    params["total_revenue"] = publisher->getTotalRevenue();
    params["joined_at"] = publisher->getJoinedAt().toString(Qt::ISODate);

    return DatabaseManager::instance()->executeQuery(query, params);
}


/*
bool UserRepository::deleteFromDatabase(int userId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) return false;
    QString deletePublisher = "DELETE FROM publisher_info WHERE user_id = :user_id";
    QVariantMap params;
    params["user_id"] = userId;
    db->executeQuery(deletePublisher, params);

    QString deleteAdmin = "DELETE FROM admin_info WHERE user_id = :user_id";
    db->executeQuery(deleteAdmin, params);

    QString deleteUser = "DELETE FROM user WHERE id = :user_id";
    return db->executeQuery(deleteUser, params);
}
*/




bool UserRepository::deleteFromDatabase(int userId) {
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "deleteFromDatabase: Database is not open!";
        return false;
    }

    // ✅ بررسی شروع تراکنش
    if (!db->transaction()) {
        qCritical() << "deleteFromDatabase: Failed to start transaction for user" << userId;
        return false;
    }

    QVariantMap params;
    params["user_id"] = userId;

    QString deleteUser = "DELETE FROM user WHERE id = :user_id";
    bool ok = db->executeQuery(deleteUser, params);

    if (ok) {
        if (!db->commit()) {
            qCritical() << "deleteFromDatabase: Commit failed!";
            db->rollback();
            return false;
        }
        qDebug() << "✅ User" << userId << "deleted";
        return true;
    }

    // ❌ خطا - Rollback
    if (!db->rollback()) {
        qCritical() << "deleteFromDatabase: Rollback failed!";
    }
    qWarning() << "❌ Failed to delete user" << userId;
    return false;
}


void UserRepository::addToCache(User* user)
{
    if (!user) return;

    usersById[user->getId()] = user;
    usersByUsername[user->getUsername()] = user;
    usersByEmail[user->getEmail()] = user;
}




void UserRepository::removeFromCache(int userId)
{
    User* user = usersById.value(userId, nullptr);
    if (!user) return;

    usersById.remove(userId);
    usersByUsername.remove(user->getUsername());
    usersByEmail.remove(user->getEmail());
}



void UserRepository::clearCache()
{
    qDeleteAll(usersById);
    usersById.clear();
    usersByUsername.clear();
    usersByEmail.clear();
}


UserRole UserRepository::stringToRole(const QString& roleStr)
{
    if (roleStr == "Admin") return UserRole::Admin;
    if (roleStr == "Publisher") return UserRole::Publisher;
    if (roleStr == "User") return UserRole::User;
    return UserRole::User;  // Default
}



AccountStatus UserRepository::stringToStatus(const QString& statusStr)
{
    if (statusStr == "Active") return AccountStatus::Active;
    if (statusStr == "Blocked") return AccountStatus::Blocked;
    if (statusStr == "Inactive") return AccountStatus::Inactive;
    if (statusStr == "Suspended") return AccountStatus::Suspended;
    if (statusStr == "Pending") return AccountStatus::Pending;
    return AccountStatus::Active;  // Default
}

// ===== Convert UserRole to String (برای ذخیره در دیتابیس) =====
QString UserRepository::roleToString(UserRole role)
{
    switch(role) {
    case UserRole::Admin: return "Admin";
    case UserRole::Publisher: return "Publisher";
    case UserRole::User: return "User";
    default: return "User";
    }
}

// ===== Convert AccountStatus to String (برای ذخیره در دیتابیس) =====
QString UserRepository::statusToString(AccountStatus status)
{
    switch(status) {
    case AccountStatus::Active: return "Active";
    case AccountStatus::Blocked: return "Blocked";
    case AccountStatus::Inactive: return "Inactive";
    case AccountStatus::Suspended: return "Suspended";
    case AccountStatus::Pending: return "Pending";
    default: return "Active";
    }
}



QVector<Genre> UserRepository::stringToGenres(const QString& str) const {
    QVector<Genre> genres;

    // اگر رشته خالی است
    if (str.isEmpty()) {
        return genres;
    }

    // جدا کردن بر اساس کاما
    QStringList genreStrings = str.split(",", Qt::SkipEmptyParts);

    for (const QString& genreStr : genreStrings) {
        QString trimmed = genreStr.trimmed();
        Genre genre = GenreHelper::fromString(trimmed);
        if (genre != Genre::other || trimmed == "Other") {
            genres.append(genre);
        }
    }

    return genres;
}


QString UserRepository::genresToString(const QVector<Genre>& genres) const {
    if (genres.isEmpty()) {
        return QString();  // یا return "Other";
    }

    QStringList genreStrings;

    for (Genre genre : genres) {
        switch (genre) {
        case Genre::Adventure:               genreStrings << "Adventure"; break;
        case Genre::Art:                     genreStrings << "Art"; break;
        case Genre::Biography:               genreStrings << "Biography"; break;
        case Genre::Comedy:                  genreStrings << "Comedy"; break;
        case Genre::Comics:                  genreStrings << "Comics"; break;
        case Genre::Cooking:                 genreStrings << "Cooking"; break;
        case Genre::Documentation:           genreStrings << "Documentation"; break;
        case Genre::Drama:                   genreStrings << "Drama"; break;
        case Genre::Education:               genreStrings << "Education"; break;
        case Genre::Fantasy:                 genreStrings << "Fantasy"; break;
        case Genre::Fiction:                 genreStrings << "Fiction"; break;
        case Genre::Health:                  genreStrings << "Health"; break;
        case Genre::History:                 genreStrings << "History"; break;
        case Genre::Horror:                  genreStrings << "Horror"; break;
        case Genre::Language_Learning:       genreStrings << "Language_Learning"; break;
        case Genre::LGBTQ:                   genreStrings << "LGBTQ"; break;
        case Genre::Manga:                   genreStrings << "Manga"; break;
        case Genre::Music:                   genreStrings << "Music"; break;
        case Genre::Mystery_and_Crime:       genreStrings << "Mystery_and_Crime"; break;
        case Genre::Personal_Development:    genreStrings << "Personal_Development"; break;
        case Genre::Philosophy:              genreStrings << "Philosophy"; break;
        case Genre::Poetry:                  genreStrings << "Poetry"; break;
        case Genre::Politics_and_Society:    genreStrings << "Politics_and_Society"; break;
        case Genre::Psychology:              genreStrings << "Psychology"; break;
        case Genre::Reference:                genreStrings << "Refrence"; break;
        case Genre::Romance:                 genreStrings << "Romance"; break;
        case Genre::Science:                 genreStrings << "Science"; break;
        case Genre::Science_Fiction:         genreStrings << "Science_Fiction"; break;
        case Genre::Technology:              genreStrings << "Technology"; break;
        case Genre::Thriller:                genreStrings << "Thriller"; break;
        case Genre::other:                   genreStrings << "other"; break;
        default:                             genreStrings << "other"; break;
        }
    }
    return genreStrings.join(", ");
}




void UserRepository::loadAdminInfo(Admin* admin) {
    if (!admin) {
        qWarning() << "loadAdminInfo: admin is null!";
        return;
    }
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) {
        qWarning() << "loadAdminInfo: Database is not open!";
        return;
    }
    QString query = "SELECT * FROM admin_info WHERE user_id = :user_id";
    QVariantMap params;
    params["user_id"] = admin->getId();

    QSqlQuery result = db->executeSelect(query, params);




    if (result.lastError().isValid()) {
        qWarning() << "loadAdminInfo: Query failed for user_id"
                   << admin->getId() << ":" << result.lastError().text();
        return;
    }


    if (result.next()) {
        // تبدیل رشته به AdminLevel
        QString levelStr = result.value("admin_level").toString();
        admin->setAdminLevel(stringToAdminLevel(levelStr));

        // last_action
        QString lastActionStr = result.value("last_action").toString();
        if (!lastActionStr.isEmpty()) {
            admin->setLastAction(
                QDateTime::fromString(lastActionStr, Qt::ISODate)
                );
        }
        qDebug() << "✅ Loaded admin info for user" << admin->getId();
    }else {
        qDebug() << "No admin info found for user" << admin->getId();
    }
}


AdminLevel UserRepository::stringToAdminLevel(const QString& str) const {
    if (str == "Admin") return AdminLevel::Admin;
    if (str == "SuperAdmin") return AdminLevel::SuperAdmin;
    if (str == "Moderator") return AdminLevel::Moderator;

    return AdminLevel::Admin;  // مقدار پیش‌فرض
}




bool UserRepository::saveAdminInfo(Admin* admin) {
    if (!admin) return false;

    DatabaseManager* db = DatabaseManager::instance();
    if (!db->isOpen()) return false;

    QString query = R"(
        INSERT OR REPLACE INTO admin_info (
            user_id, admin_level, last_action
        ) VALUES (
            :user_id, :admin_level, :last_action
        )
    )";

    QVariantMap params;
    params["user_id"] = admin->getId();
    params["admin_level"] = adminLevelToString(admin->getAdminLevel());
    params["last_action"] = admin->getLastAction().toString(Qt::ISODate);

    return db->executeQuery(query, params);
}


QString UserRepository::adminLevelToString(AdminLevel level) const {
    switch(level) {
    case AdminLevel::Admin:     return "Admin";
    case AdminLevel::SuperAdmin: return "SuperAdmin";
    case AdminLevel::Moderator:    return "Moderator";

    default: return "Admin";
    }
}



