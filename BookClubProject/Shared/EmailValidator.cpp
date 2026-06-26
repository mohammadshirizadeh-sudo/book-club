// emailvalidator.cpp
#include "EmailValidator.h"
#include <QDebug>

// ===== Static member definition =====
QString EmailValidator::lastError = "";

// ===== Private Methods =====

bool EmailValidator::isTypo(const QString& domain, const QString& correct) {
    if (domain == correct) return false;

    int diffCount = 0;
    for (int i = 0; i < qMin(domain.length(), correct.length()); ++i) {
        if (domain[i] != correct[i]) {
            diffCount++;
        }
    }

    if (diffCount == 2 && domain.length() == correct.length()) {
        for (int i = 0; i < domain.length() - 1; ++i) {
            if (domain[i] == correct[i+1] && domain[i+1] == correct[i]) {
                return true;
            }
        }
    }

    if (qAbs(domain.length() - correct.length()) > 1) return false;
    if (diffCount > 1) return false;

    return true;
}

bool EmailValidator::isCommonTypo(const QString& domain) {
    QStringList commonDomains = {
        "gmail.com", "yahoo.com", "outlook.com", "hotmail.com",
        "icloud.com", "protonmail.com", "proton.me", "yandex.com"
    };

    QString d = domain.toLower();

    for (const QString& correct : commonDomains) {
        if (isTypo(d, correct)) {
            return true;
        }
    }

    return false;
}

// ===== Public Methods =====

bool EmailValidator::isValid(const QString& email) {
    if (email.isEmpty()) {
        lastError = "Email address cannot be empty";
        return false;
    }

    QRegularExpression emailRegex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
        );

    if (!emailRegex.match(email).hasMatch()) {
        lastError = "Invalid email format";
        return false;
    }

    if (email.contains("..")) {
        lastError = "Email cannot contain consecutive dots";
        return false;
    }

    if (email.startsWith('.')) {
        lastError = "Email cannot start with a dot";
        return false;
    }

    if (email.contains(" ")) {
        lastError = "Email cannot contain spaces";
        return false;
    }

    QStringList parts = email.split('@');
    if (parts.size() != 2) {
        lastError = "Invalid email structure";
        return false;
    }

    QString domain = parts[1];
    if (domain.isEmpty() || !domain.contains('.')) {
        lastError = "Invalid domain name";
        return false;
    }

    if (isCommonTypo(domain)) {
        lastError = "Possible typo in email domain";
        return false;
    }

    lastError = "";
    return true;
}

bool EmailValidator::isDisposableEmail(const QString& email) {
    QStringList disposableDomains = {
        "tempmail.com", "throwaway.com", "guerrillamail.com",
        "mailinator.com", "trashmail.com", "spamgourmet.com"
    };

    QString domain = email.split('@').last().toLower();
    return disposableDomains.contains(domain);
}

QString EmailValidator::getDomain(const QString& email) {
    if (email.isEmpty() || !email.contains('@')) {
        return "";
    }
    return email.split('@').last();
}

QString EmailValidator::getUsername(const QString& email) {
    if (email.isEmpty() || !email.contains('@')) {
        return "";
    }
    return email.split('@').first();
}

QString EmailValidator::getLastError() {
    return lastError;
}

QString EmailValidator::suggestCorrection(const QString& email) {
    if (!email.contains('@')) {
        return "";
    }

    QString domain = getDomain(email);
    QStringList commonDomains = {
        "gmail.com", "yahoo.com", "outlook.com", "hotmail.com"
    };

    for (const QString& correct : commonDomains) {
        if (isTypo(domain, correct)) {
            QString username = getUsername(email);
            return username + "@" + correct;
        }
    }

    return "";
}

bool EmailValidator::isValidForLogin(const QString& email) {
    return isValid(email) && !isDisposableEmail(email);
}