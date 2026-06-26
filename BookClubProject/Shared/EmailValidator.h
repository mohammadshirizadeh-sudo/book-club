// emailvalidator.h
#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QString>
#include <QRegularExpression>

class EmailValidator {
private:
    static QString lastError;
    static bool isTypo(const QString& domain, const QString& correct);
    static bool isCommonTypo(const QString& domain);

public:
    static bool isValid(const QString& email);
    static bool isDisposableEmail(const QString& email);
    static QString getDomain(const QString& email);
    static QString getUsername(const QString& email);
    static QString getLastError();
    static QString suggestCorrection(const QString& email);
    static bool isValidForLogin(const QString& email);
};

#endif // EMAILVALIDATOR_H