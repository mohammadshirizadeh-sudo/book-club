// emailvalidator.h
#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QString>
#include <QRegularExpression>


class EmailValidator {
private:
    static QString lastError;
    static bool isTypo(const QString& domain, const QString& correct) {//i mean if user type forexample gmial.com instead of gmail.com
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

        // Allow for one missing/extra character
        if (qAbs(domain.length() - correct.length()) > 1) return false;
        if (diffCount > 1) return false;

        return true;
    }


    static bool isCommonTypo(const QString& domain) {
        QStringList commonDomains = {
            "gmail.com", "yahoo.com", "outlook.com", "hotmail.com",
            "icloud.com", "protonmail.com", "proton.me", "yandex.com"
        };

        QString d = domain.toLower();

        // Check for typos like "gmial.com" instead of "gmail.com"
        for (const QString& correct : commonDomains) {
            if (isTypo(d, correct)) {
                return true;
            }
        }

        return false;
    }



public:


    static bool isValid(const QString& email) {
        // Check if email is empty
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

        // Additional checks for common invalid patterns
        if (email.contains("..")) {
            lastError = "Email cannot contain consecutive dots";
            return false;
        }

        if (email.startsWith('.')) {
            lastError = "Email cannot start or end with a dot";
            return false;
        }

        if (email.contains(" ")) {
            lastError = "Email cannot contain spaces";
            return false;
        }

        // Check for valid domain (basic)
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

        // Optional: Check for common typos in domain

        if (isCommonTypo(domain)) {
            lastError = "Possible typo in email domain";
            return false;
        }

        lastError = "";
        return true;
    }


    static bool isDisposableEmail(const QString& email) {
        QStringList disposableDomains = {
            "tempmail.com", "throwaway.com", "guerrillamail.com",
            "mailinator.com", "trashmail.com", "spamgourmet.com"
        };

        QString domain = email.split('@').last().toLower();
        return disposableDomains.contains(domain);
    }

    static QString getDomain(const QString& email) {
        if (email.isEmpty() || !email.contains('@')) {
            return "";
        }
        return email.split('@').last();
    }


    static QString getUsername(const QString& email) {
        if (email.isEmpty() || !email.contains('@')) {
            return "";
        }
        return email.split('@').first();
    }



    static QString getLastError() {
        return lastError;
    }


    static QString suggestCorrection(const QString& email) {
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


    static bool isValidForLogin(const QString& email) {//that is the final function u can use it
        return isValid(email) && !isDisposableEmail(email);
    }
};

// Static member initialization
QString EmailValidator::lastError = "";

#endif // EMAILVALIDATOR_H