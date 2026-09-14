#pragma once

#include <stdexcept>
#include <string>

class InvalidStatisticsException : public std::runtime_error {
public:
    /** Creates a validation error with a message safe to return as INVALID_ARGUMENT. */
    explicit InvalidStatisticsException(const std::string& message)
        : std::runtime_error{message} {}
};
