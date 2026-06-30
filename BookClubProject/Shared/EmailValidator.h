// emailvalidator.h
#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QString>
#include <QRegularExpression>
#include "ValidationResult.h"

class EmailValidator {
private:

    static bool isTypo(const QString& domain, const QString& correct);
    static bool isCommonTypo(const QString& domain);

public:
    static ValidationResult isValidForLogin(const QString& email);
    static ValidationResult isValid(const QString& email);
    static bool isDisposableEmail(const QString& email);
    static QString getDomain(const QString& email);
    static QString getUsername(const QString& email);

    static QString suggestCorrection(const QString& email);
};

#endif // EMAILVALIDATOR_H