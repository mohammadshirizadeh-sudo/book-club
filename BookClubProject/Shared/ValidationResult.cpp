
#include "ValidationResult.h"

ValidationResult::ValidationResult()
    : isValid(true)
    , errorMessage("") {
}

ValidationResult::ValidationResult(bool valid, const QString& msg)
    : isValid(valid)
    , errorMessage(msg) {
}

ValidationResult ValidationResult::success() {
    return ValidationResult(true, "");
}

ValidationResult::ValidationResult(bool valid, const QString& msg, const QVariantMap& d)
    : isValid(valid)
    , errorMessage(msg)
    , data(d) {
}

ValidationResult ValidationResult::success(const QVariantMap& data) {
    return ValidationResult(true, "", data);
}

ValidationResult ValidationResult::failure(const QString& msg) {
    return ValidationResult(false, msg);
}
QVariantMap ValidationResult::getData() const
{
    return data;
}
