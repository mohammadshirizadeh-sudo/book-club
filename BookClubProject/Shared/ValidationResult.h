
#ifndef VALIDATIONRESULT_H
#define VALIDATIONRESULT_H

#include <QString>

struct ValidationResult {
    bool isValid;
    QString errorMessage;

    ValidationResult();
    ValidationResult(bool valid, const QString& msg);

    static ValidationResult success();
    static ValidationResult failure(const QString& msg);
};

#endif