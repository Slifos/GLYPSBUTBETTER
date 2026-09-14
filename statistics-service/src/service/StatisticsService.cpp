#include "service/StatisticsService.hpp"

#include "exception/InvalidStatisticsException.hpp"

namespace campusconnect::statistics::service {

model::Statistics StatisticsService::calculate(
    const int maxParticipants,
    const int currentParticipants) const {
    if (maxParticipants <= 0) {
        throw exception::InvalidStatisticsException{
            "maxParticipants must be greater than 0"};
    }

    if (currentParticipants < 0) {
        throw exception::InvalidStatisticsException{
            "currentParticipants cannot be negative"};
    }

    const int remainingPlaces = currentParticipants >= maxParticipants
                                    ? 0
                                    : maxParticipants - currentParticipants;
    const double occupancyRate =
        static_cast<double>(currentParticipants) /
        static_cast<double>(maxParticipants) * 100.0;

    return model::Statistics{
        .remainingPlaces = remainingPlaces,
        .occupancyRate = occupancyRate,
        .full = currentParticipants >= maxParticipants,
    };
}

}  // namespace campusconnect::statistics::service
