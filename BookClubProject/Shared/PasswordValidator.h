// passwordvalidator.h
#ifndef PASSWORDVALIDATOR_H
#define PASSWORDVALIDATOR_H

#include <QString>
#include <QRegularExpression>

enum class PasswordStrength {
    Weak,
    Medium,
    Strong,
    VeryStrong
};

class PasswordValidator {
private:
    static QString lastError;
    static bool isCommonPassword(const QString& password);

public:
    static bool isValid(const QString& password);
    static PasswordStrength checkStrength(const QString& password);
    static QString getStrengthString(PasswordStrength strength);
    static QString getLastError();
};

#endif // PASSWORDVALIDATOR_H