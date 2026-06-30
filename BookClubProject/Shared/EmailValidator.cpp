// emailvalidator.cpp
#include "EmailValidator.h"
#include "ValidationResult.h"
#include <QDebug>


// ===== Private Methods =====

bool EmailValidator::isTypo(const QString& domain, const QString& correct) {
    if (domain == correct) return false;

    if (qAbs(domain.length() - correct.length()) > 1) return false;
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

ValidationResult EmailValidator::isValid(const QString& email) {
    if (email.isEmpty()) {
        return ValidationResult::failure("Email address cannot be empty");
    }

    QRegularExpression emailRegex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
        );

    if (!emailRegex.match(email).hasMatch()) {
        return ValidationResult::failure("Invalid email format");

    }

    if (email.contains("..")) {
        return ValidationResult::failure("Email cannot contain consecutive dots");

    }

    if (email.startsWith('.')) {
        return ValidationResult::failure("Email cannot start with a dot");

    }

    if (email.contains(" ")) {
        return ValidationResult::failure("Email cannot contain spaces");
    }

    QStringList parts = email.split('@');
    if (parts.size() != 2) {
        return ValidationResult::failure("Invalid email structure");
    }

    QString domain = parts[1];
    if (domain.isEmpty() || !domain.contains('.')) {
        return ValidationResult::failure("Invalid domain name");
    }

    if (isCommonTypo(domain)) {
        return ValidationResult::failure("Possible typo in email domain");
    }
    return ValidationResult::success();
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
ValidationResult EmailValidator::isValidForLogin(const QString& email) {
    // 1. اول اعتبارسنجی فرمت
    ValidationResult basicResult = isValid(email);
    if (!basicResult.isValid) {
        return basicResult;  // ← خطای فرمت را برمی‌گرداند
    }

    // 2. سپس بررسی ایمیل یکبارمصرف
    if (isDisposableEmail(email)) {
        return ValidationResult::failure("Disposable/temporary email addresses are not allowed");
    }

    return ValidationResult::success();
}

