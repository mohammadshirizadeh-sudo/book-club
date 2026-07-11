
#ifndef VALIDATIONRESULT_H
#define VALIDATIONRESULT_H

#include <QString>
#include <QVariantMap>

struct ValidationResult {
    bool isValid;
    QString errorMessage;
    QVariantMap data;

    ValidationResult();
    ValidationResult(bool valid, const QString& msg);

    ValidationResult(bool valid, const QString& msg, const QVariantMap& data);

    static ValidationResult success();
    static ValidationResult success(const QVariantMap& data);
    static ValidationResult failure(const QString& msg);

public:
    QVariantMap getData() const;
};

#endif