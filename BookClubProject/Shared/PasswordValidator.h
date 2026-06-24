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

public:

    static bool isValid(const QString& password) {

        if (password.length() < 8) {
            lastError = "Password must be at least 8 characters long";
            return false;
        }

        if (password.contains(" ")) {
            lastError = "Password must not contain spaces";
            return false;
        }

        // Check for common weak passwords (optional but recommended)
        if (isCommonPassword(password)) {
            lastError = "This password is too common and easily guessable";
            return false;
        }

        lastError = "";
        return true;
    }

    static PasswordStrength checkStrength(const QString& password) {
        int score = 0;


        if (password.length() >= 8) score++;
        if (password.length() >= 12) score++;
        if (password.length() >= 16) score++;


        if (password.contains(QRegularExpression("[A-Z]"))) score++;
        if (password.contains(QRegularExpression("[a-z]"))) score++;
        if (password.contains(QRegularExpression("[0-9]"))) score++;
        if (password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) score++;

        // Bonus for having all character types
        if (password.length() >= 8 &&
            password.contains(QRegularExpression("[A-Z]")) &&
            password.contains(QRegularExpression("[a-z]")) &&
            password.contains(QRegularExpression("[0-9]")) &&
            password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) {
            score += 2;
        }

        // Determine strength level
        if (score <= 3) return PasswordStrength::Weak;
        if (score <= 5) return PasswordStrength::Medium;
        if (score <= 7) return PasswordStrength::Strong;
        return PasswordStrength::VeryStrong;
    }

    static bool isCommonPassword(const QString& password) {
        QStringList commonPasswords = {
            "12345678", "password", "123456789", "qwerty123",
            "admin123", "letmein", "welcome", "password123"
        };
        return commonPasswords.contains(password.toLower());
    }

    static QString getStrengthString(PasswordStrength strength) {
        switch(strength) {
        case PasswordStrength::Weak:       return "Weak";
        case PasswordStrength::Medium:     return "Medium";
        case PasswordStrength::Strong:     return "Strong";
        case PasswordStrength::VeryStrong: return "Very Strong";
        default: return "Unknown";
        }
    }

    static QString getLastError() {
        return lastError;
    }
};

QString PasswordValidator::lastError = "";

#endif // PASSWORDVALIDATOR_H