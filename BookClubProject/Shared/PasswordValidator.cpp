// passwordvalidator.cpp
#include "PasswordValidator.h"
#include <QDebug>





// ===== Static member definition =====

QString PasswordValidator::lastError = "";

// ===== Private Methods =====

bool PasswordValidator::isCommonPassword(const QString& password) {
    QStringList commonPasswords = {
        "12345678", "password", "123456789", "qwerty123",
        "admin123", "letmein", "welcome", "password123"
    };
    return commonPasswords.contains(password.toLower());
}

// ===== Public Methods =====

bool PasswordValidator::isValid(const QString& password) {
    if (password.length() < 8) {
        lastError = "Password must be at least 8 characters long";
        return false;
    }

    if (password.contains(" ")) {
        lastError = "Password must not contain spaces";
        return false;
    }

    if (isCommonPassword(password)) {
        lastError = "This password is too common and easily guessable";
        return false;
    }

    lastError = "";
    return true;
}

PasswordStrength PasswordValidator::checkStrength(const QString& password) {
    int score = 0;

    if (password.length() >= 8) score++;
    if (password.length() >= 12) score++;
    if (password.length() >= 16) score++;

    if (password.contains(QRegularExpression("[A-Z]"))) score++;
    if (password.contains(QRegularExpression("[a-z]"))) score++;
    if (password.contains(QRegularExpression("[0-9]"))) score++;
    if (password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) score++;

    if (password.length() >= 8 &&
        password.contains(QRegularExpression("[A-Z]")) &&
        password.contains(QRegularExpression("[a-z]")) &&
        password.contains(QRegularExpression("[0-9]")) &&
        password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) {
        score += 2;
    }

    if (score <= 3) return PasswordStrength::Weak;
    if (score <= 5) return PasswordStrength::Medium;
    if (score <= 7) return PasswordStrength::Strong;
    return PasswordStrength::VeryStrong;
}

QString PasswordValidator::getStrengthString(PasswordStrength strength) {
    switch(strength) {
    case PasswordStrength::Weak:       return "Weak";
    case PasswordStrength::Medium:     return "Medium";
    case PasswordStrength::Strong:     return "Strong";
    case PasswordStrength::VeryStrong: return "Very Strong";
    default: return "Unknown";
    }
}

QString PasswordValidator::getLastError() {
    return lastError;
}