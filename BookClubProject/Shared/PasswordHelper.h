// passwordhelper.h
#ifndef PASSWORDHELPER_H
#define PASSWORDHELPER_H

#include <QString>
#include <QCryptographicHash>
#include <QUuid>


class PasswordHelper {
public:

    static QString hashPassword(const QString& password, const QString& salt = "") {
        QString combined = password + salt;
        QByteArray hash = QCryptographicHash::hash(combined.toUtf8(),QCryptographicHash::Sha256);
        return QString(hash.toHex());
    }

    static QString generateSalt() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }


    static bool verifyPassword(const QString& password,const QString& hashedPassword,const QString& salt = "") {
        // Hash the input password with the same salt
        QString computedHash = hashPassword(password, salt);

        // Compare with stored hash
        return computedHash == hashedPassword;
    }
};

#endif // PASSWORDHELPER_H