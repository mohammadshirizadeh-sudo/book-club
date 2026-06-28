
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

ValidationResult ValidationResult::failure(const QString& msg) {
    return ValidationResult(false, msg);
}