// passwordvalidator.h
#ifndef PASSWORDVALIDATOR_H
#define PASSWORDVALIDATOR_H

#include <QString>
#include <QRegularExpression>
#include "ValidationResult.h"

enum class PasswordStrength {
    Weak,
    Medium,
    Strong,
    VeryStrong
};

class PasswordValidator {
private:

    static bool isCommonPassword(const QString& password);

public:
    static ValidationResult isValid(const QString& password);
    static PasswordStrength checkStrength(const QString& password);
    static QString getStrengthString(PasswordStrength strength);
};

#endif // PASSWORDVALIDATOR_H