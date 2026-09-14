#pragma once

#include "model/Statistics.hpp"

namespace campusconnect::statistics::service {

class StatisticsService {
public:
    [[nodiscard]] model::Statistics calculate(
        int maxParticipants,
        int currentParticipants) const;
};

}  // namespace campusconnect::statistics::service
