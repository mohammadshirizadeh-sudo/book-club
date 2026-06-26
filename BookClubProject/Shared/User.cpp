#include "User.h"
#include "admin.h"
#include "Publisher.h"





//These constructors are for sign up












UserRole User::getRole() const
{
    return role;
}

void User::setRole(UserRole newRole)
{
    role = newRole;
    updatedAt = QDateTime::currentDateTime();
}

User::User()
    : id(0)
    , Fullname("")
    , username("")
    , passwordHash("")
    , email("")
    , role(UserRole::User)
    , status(AccountStatus::Active)
    , createdAt(QDateTime::currentDateTime())
    , favouriteGenre(QVector<QString>())
    , lastLogin(QDateTime())
    , updatedAt(QDateTime::currentDateTime())
{

}


User::User(int id, const QString& username, const QString& email, const QString& plainPassword)
    : id(id), username(username), email(email)
    , role(UserRole::User)
    , status(AccountStatus::Active)
    , createdAt(QDateTime::currentDateTime())

{
    // اعتبارسنجی پسورد
    if (!PasswordValidator::isValid(plainPassword)) {
        throw std::invalid_argument(
            "Invalid password: " +
            PasswordValidator::getLastError().toStdString()
            );
    }

    // use hash
    salt = PasswordHelper::generateSalt();
    passwordHash = PasswordHelper::hashPassword(plainPassword, salt);
}

User :: User(int id, const QString &fullName, const QString& username, const QString& email,UserRole role, AccountStatus status,
            const QDateTime& createdAt, const QDateTime & lastLogin ,const QString& plainPassword): id(id), Fullname(fullName), username(username), email(email)
    , role(role), status(status), createdAt(createdAt) , lastLogin(lastLogin), updatedAt(QDateTime::currentDateTime())
{
    if (!PasswordValidator::isValid(plainPassword)) {
        throw std::invalid_argument(
            "Invalid password: " +
            PasswordValidator::getLastError().toStdString()
            );
    }

    // تولید Salt و هش کردن
    salt = PasswordHelper::generateSalt();
    passwordHash = PasswordHelper::hashPassword(plainPassword, salt);

}


// this constructor is for database
User :: User(int id,const QString &fullName, const QString& username, const QString& _email,UserRole role, AccountStatus status,
            const QDateTime& createdAt,const QDateTime &lastLogin ,const QString& passwordHash , QVector<QString> favouriteGenre , const QDateTime& updatedAt , QString salt)
    : id(id)
    ,Fullname(fullName) , username(username) , passwordHash(passwordHash), email(_email)
    , role(role), status(status), createdAt(createdAt),favouriteGenre(favouriteGenre) ,lastLogin(lastLogin),  updatedAt(updatedAt) , salt(salt)

{

    if (passwordHash.isEmpty()) {
        qWarning() << "User loaded with empty password hash!";
    }
}











//getter and setter

int User::getId() const
{
    return id;
}

void User::setId(int newId)
{
    id = newId;
}

QString User::getUsername() const
{
    return username;
}

void User::setUsername(const QString &newUsername)
{
    username = newUsername;
}



QString User::getEmail() const
{
    return email;
}

void User::setEmail(const QString &newEmail)
{
    email = newEmail;
}


QDateTime User::getLastLogin() const
{
    return lastLogin;
}

void User::setLastLogin(const QDateTime &newLastLogin)
{
    lastLogin = newLastLogin;
}


AccountStatus User::getStatus() const
{
    return status;
}

void User::setStatus(AccountStatus newStatus)
{
    status = newStatus;
}


















QString User::getFullname() const
{
    return Fullname;
}

void User::setFullname(const QString &newFullname)
{
    Fullname = newFullname;
}


QVector<QString> User::getFavouriteGenre() const
{
    return favouriteGenre;
}

void User::setFavouriteGenre(const QVector<QString> &newFavouriteGenre)
{
    favouriteGenre = newFavouriteGenre;
}

QDateTime User::getUpdatedAt() const
{
    return updatedAt;
}

void User::setUpdatedAt(const QDateTime &newUpdatedAt)
{
    updatedAt = newUpdatedAt;
}


bool User::setPassword(const QString& password) {
    // 1. Validate password strength
    if (!PasswordValidator::isValid(password)) {
        qWarning() << "Invalid password:" << PasswordValidator::getLastError();
        return false;
    }

    // 2. Generate new salt


    // 3. Hash password with salt
    passwordHash = PasswordHelper::hashPassword(password, salt);

    // 4. Update timestamp
    updatedAt = QDateTime::currentDateTime();

    return true;
}

bool User::isAdmin() const
{
    if(this->role == UserRole::Admin)return true;
    return false;
}

bool User::isPublisher() const
{
    if (role == UserRole::Publisher)return true;
    return false;
}






bool User::isBlocked() const
{
    if(status == AccountStatus::Blocked)return true;
    return false;
}


bool User::checkPassword(const QString& password) const
{
    // رمز ورودی رو با salt ذخیره‌شده هش کن
    QString hashPass = PasswordHelper::hashPassword(password, salt);

    // هش جدید رو با هش ذخیره‌شده مقایسه کن
    return hashPass == passwordHash;
}




User* User::createUser(int id, const QString& username, const QString& email,
                       const QString& password, UserRole role) {
    switch(role) {
    case UserRole::Admin:
        return new Admin(id, username, email, password);
    case UserRole::Publisher:
        return new Publisher(id, username, email, password , "");
    default:
        return new User(id, username, email, password);
    }
}










