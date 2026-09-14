#pragma once

#include <stdexcept>
#include <string>

namespace campusconnect::statistics::exception {

class InvalidStatisticsException : public std::runtime_error {
public:
    explicit InvalidStatisticsException(const std::string& message)
        : std::runtime_error{message} {}
};

}  // namespace campusconnect::statistics::exception
