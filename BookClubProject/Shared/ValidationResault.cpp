
#include "ValidationResult.h"

ValidationResult::ValidationResult()
    : isValid(true)
    , errorMessage("")
    ,data(QVariantMap())
{
}

ValidationResult::ValidationResult(bool valid, const QString& msg)
    : isValid(valid)
    , errorMessage(msg)
    , data(QVariantMap())
{
}

ValidationResult::ValidationResult(bool valid, const QString& msg, const QVariantMap& data)
    : isValid(valid)
    , errorMessage(msg)
    , data(data)
{
}

ValidationResult ValidationResult::success() {
    return ValidationResult(true, "");
}

ValidationResult ValidationResult::success(const QVariantMap& data)
{
    return ValidationResult(true, "", data);
}

ValidationResult ValidationResult::failure(const QString& msg) {
    return ValidationResult(false, msg);
}