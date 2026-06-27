// validationresult.h
#ifndef VALIDATIONRESULT_H
#define VALIDATIONRESULT_H

#include <QString>

struct ValidationResult {
    bool isValid;
    QString errorMessage;

    ValidationResult() : isValid(true), errorMessage("") {}
    ValidationResult(bool valid, const QString& msg) : isValid(valid), errorMessage(msg) {}

    static ValidationResult success() {
        return ValidationResult(true, "");
    }

    static ValidationResult failure(const QString& msg) {
        return ValidationResult(false, msg);
    }
};

#endif // VALIDATIONRESULT_H