#include "service/StatisticsService.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include "exception/InvalidStatisticsException.hpp"

namespace campusconnect::statistics::service {

namespace {

constexpr std::int64_t secondsPerDay = 86'400;

// Enforces the invariants shared by detailed and dashboard analyses.
void validateEvent(
    const model::EventData& event,
    const std::int64_t asOfEpochSeconds) {
    if (asOfEpochSeconds <= 0) {
        throw exception::InvalidStatisticsException{
            "asOfEpochSeconds must be greater than 0"};
    }
    if (event.eventId <= 0) {
        throw exception::InvalidStatisticsException{
            "eventId must be greater than 0"};
    }
    if (event.eventType.empty()) {
        throw exception::InvalidStatisticsException{
            "eventType cannot be empty"};
    }
    if (event.maxParticipants <= 0) {
        throw exception::InvalidStatisticsException{
            "maxParticipants must be greater than 0"};
    }
    if (event.currentParticipants < 0) {
        throw exception::InvalidStatisticsException{
            "currentParticipants cannot be negative"};
    }
    if (event.cancelledParticipants < 0) {
        throw exception::InvalidStatisticsException{
            "cancelledParticipants cannot be negative"};
    }
    if (event.attendedParticipants < 0) {
        throw exception::InvalidStatisticsException{
            "attendedParticipants cannot be negative"};
    }
    if (event.attendedParticipants > event.currentParticipants) {
        throw exception::InvalidStatisticsException{
            "attendedParticipants cannot exceed currentParticipants"};
    }

    for (const auto& record : event.registrationHistory) {
        if (record.timestampEpochSeconds < 0) {
            throw exception::InvalidStatisticsException{
                "registration timestamp cannot be negative"};
        }
        if (record.timestampEpochSeconds > asOfEpochSeconds) {
            throw exception::InvalidStatisticsException{
                "registration timestamp cannot be after asOfEpochSeconds"};
        }
        if (record.registrations <= 0) {
            throw exception::InvalidStatisticsException{
                "registration count must be greater than 0"};
        }
    }
}

// Groups timestamped registrations into ordered UTC day or Monday-based week buckets.
std::vector<model::RegistrationBucket> aggregateRegistrations(
    const std::vector<model::RegistrationRecord>& history,
    const bool weekly) {
    std::map<std::int64_t, std::int64_t> registrationsByPeriod;

    for (const auto& record : history) {
        const std::int64_t day = record.timestampEpochSeconds / secondsPerDay;
        // Unix day zero was a Thursday, hence +3 when finding Monday.
        const std::int64_t periodDay =
            weekly ? day - ((day + 3) % 7) : day;
        registrationsByPeriod[periodDay * secondsPerDay] +=
            record.registrations;
    }

    std::vector<model::RegistrationBucket> buckets;
    buckets.reserve(registrationsByPeriod.size());
    for (const auto& [periodStart, registrations] : registrationsByPeriod) {
        if (registrations > std::numeric_limits<int>::max()) {
            throw exception::InvalidStatisticsException{
                "registration bucket exceeds the supported range"};
        }
        buckets.push_back(model::RegistrationBucket{
            .periodStartEpochSeconds = periodStart,
            .registrations = static_cast<int>(registrations),
        });
    }
    return buckets;
}

// Calculates a percentage while defining an empty denominator as zero percent.
double percentage(const std::int64_t numerator, const std::int64_t denominator) {
    if (denominator == 0) {
        return 0.0;
    }
    return static_cast<double>(numerator) /
           static_cast<double>(denominator) * 100.0;
}

struct EventTypeAccumulator {
    int eventCount{};
    std::int64_t currentParticipants{};
    std::int64_t attendedParticipants{};
    std::int64_t demand{};
};

}  // namespace

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

model::EventAnalytics StatisticsService::analyzeEvent(
    const model::EventData& event,
    const std::int64_t asOfEpochSeconds) const {
    validateEvent(event, asOfEpochSeconds);

    const model::Statistics capacity =
        calculate(event.maxParticipants, event.currentParticipants);
    const std::int64_t totalRegistrations =
        static_cast<std::int64_t>(event.currentParticipants) +
        event.cancelledParticipants;
    const double cancellationRate =
        percentage(event.cancelledParticipants, totalRegistrations);
    const double attendanceRate =
        percentage(event.attendedParticipants, event.currentParticipants);

    std::int64_t recordedRegistrations = 0;
    std::int64_t earliestRegistration = asOfEpochSeconds;
    for (const auto& record : event.registrationHistory) {
        if (record.registrations >
            std::numeric_limits<std::int64_t>::max() - recordedRegistrations) {
            throw exception::InvalidStatisticsException{
                "registration history exceeds the supported range"};
        }
        recordedRegistrations += record.registrations;
        earliestRegistration =
            std::min(earliestRegistration, record.timestampEpochSeconds);
    }

    double registrationsPerDay = 0.0;
    if (!event.registrationHistory.empty()) {
        const std::int64_t observedSeconds = std::max(
            secondsPerDay, asOfEpochSeconds - earliestRegistration);
        registrationsPerDay =
            static_cast<double>(recordedRegistrations) *
            static_cast<double>(secondsPerDay) /
            static_cast<double>(observedSeconds);
    }

    std::optional<std::int64_t> estimatedFullAt;
    if (capacity.full) {
        estimatedFullAt = asOfEpochSeconds;
    } else if (registrationsPerDay > 0.0) {
        const long double secondsUntilFull = std::ceil(
            static_cast<long double>(capacity.remainingPlaces) /
            static_cast<long double>(registrationsPerDay) *
            static_cast<long double>(secondsPerDay));
        const long double maximumAdditionalSeconds =
            static_cast<long double>(std::numeric_limits<std::int64_t>::max()) -
            static_cast<long double>(asOfEpochSeconds);
        if (secondsUntilFull <= maximumAdditionalSeconds) {
            estimatedFullAt = asOfEpochSeconds +
                              static_cast<std::int64_t>(secondsUntilFull);
        }
    }

    std::vector<model::CapacityAlert> alerts;
    if (capacity.occupancyRate >= 80.0) {
        alerts.push_back(model::CapacityAlert{
            .level = model::CapacityAlertLevel::eightyPercent,
            .threshold = 80.0,
            .message = "Event capacity has reached 80%",
        });
    }
    if (capacity.occupancyRate >= 90.0) {
        alerts.push_back(model::CapacityAlert{
            .level = model::CapacityAlertLevel::ninetyPercent,
            .threshold = 90.0,
            .message = "Event capacity has reached 90%",
        });
    }
    if (capacity.full) {
        alerts.push_back(model::CapacityAlert{
            .level = model::CapacityAlertLevel::full,
            .threshold = 100.0,
            .message = "Event is full",
        });
    }

    return model::EventAnalytics{
        .eventId = event.eventId,
        .capacity = capacity,
        .cancellationRate = cancellationRate,
        .attendedParticipants = event.attendedParticipants,
        .attendanceRate = attendanceRate,
        .dailyRegistrations =
            aggregateRegistrations(event.registrationHistory, false),
        .weeklyRegistrations =
            aggregateRegistrations(event.registrationHistory, true),
        .registrationsPerDay = registrationsPerDay,
        .estimatedFullAtEpochSeconds = estimatedFullAt,
        .alerts = std::move(alerts),
    };
}

model::DashboardStatistics StatisticsService::calculateDashboard(
    const std::vector<model::EventData>& events,
    const std::int64_t asOfEpochSeconds,
    const std::size_t popularEventsLimit) const {
    if (asOfEpochSeconds <= 0) {
        throw exception::InvalidStatisticsException{
            "asOfEpochSeconds must be greater than 0"};
    }

    std::set<std::int64_t> eventIds;
    std::vector<model::EventComparison> comparisons;
    std::vector<model::PopularEvent> popularEvents;
    std::map<std::string, EventTypeAccumulator> accumulatorsByType;
    comparisons.reserve(events.size());
    popularEvents.reserve(events.size());

    std::int64_t totalCapacity = 0;
    std::int64_t totalCurrent = 0;
    std::int64_t totalCancelled = 0;
    std::int64_t totalAttended = 0;
    int fullEvents = 0;

    for (const auto& event : events) {
        if (!eventIds.insert(event.eventId).second) {
            throw exception::InvalidStatisticsException{
                "eventId values must be unique"};
        }

        const model::EventAnalytics analytics =
            analyzeEvent(event, asOfEpochSeconds);
        comparisons.push_back(model::EventComparison{
            .eventId = event.eventId,
            .eventType = event.eventType,
            .currentParticipants = event.currentParticipants,
            .occupancyRate = analytics.capacity.occupancyRate,
            .cancellationRate = analytics.cancellationRate,
            .attendanceRate = analytics.attendanceRate,
            .registrationsPerDay = analytics.registrationsPerDay,
        });
        popularEvents.push_back(model::PopularEvent{
            .eventId = event.eventId,
            .eventType = event.eventType,
            .currentParticipants = event.currentParticipants,
            .occupancyRate = analytics.capacity.occupancyRate,
            .rank = 0,
        });

        auto& type = accumulatorsByType[event.eventType];
        ++type.eventCount;
        type.currentParticipants += event.currentParticipants;
        type.attendedParticipants += event.attendedParticipants;
        type.demand += static_cast<std::int64_t>(event.currentParticipants) +
                       event.cancelledParticipants;

        totalCapacity += event.maxParticipants;
        totalCurrent += event.currentParticipants;
        totalCancelled += event.cancelledParticipants;
        totalAttended += event.attendedParticipants;
        if (analytics.capacity.full) {
            ++fullEvents;
        }
    }

    std::sort(
        popularEvents.begin(),
        popularEvents.end(),
        [](const model::PopularEvent& left, const model::PopularEvent& right) {
            if (left.currentParticipants != right.currentParticipants) {
                return left.currentParticipants > right.currentParticipants;
            }
            if (left.occupancyRate != right.occupancyRate) {
                return left.occupancyRate > right.occupancyRate;
            }
            return left.eventId < right.eventId;
        });

    if (popularEventsLimit > 0 && popularEventsLimit < popularEvents.size()) {
        popularEvents.resize(popularEventsLimit);
    }
    for (std::size_t index = 0; index < popularEvents.size(); ++index) {
        popularEvents[index].rank = static_cast<int>(index + 1);
    }

    std::vector<model::EventTypeStatistics> statisticsByType;
    statisticsByType.reserve(accumulatorsByType.size());
    for (const auto& [eventType, accumulator] : accumulatorsByType) {
        const double averageParticipants =
            static_cast<double>(accumulator.currentParticipants) /
            accumulator.eventCount;
        const double averageDemand =
            static_cast<double>(accumulator.demand) / accumulator.eventCount;
        const double recommended = std::ceil(averageDemand * 1.1);
        const int recommendedCapacity = recommended >
                                                std::numeric_limits<int>::max()
                                            ? std::numeric_limits<int>::max()
                                            : static_cast<int>(recommended);

        statisticsByType.push_back(model::EventTypeStatistics{
            .eventType = eventType,
            .eventCount = accumulator.eventCount,
            .averageParticipants = averageParticipants,
            .averageAttendanceRate = percentage(
                accumulator.attendedParticipants,
                accumulator.currentParticipants),
            .recommendedCapacity = recommendedCapacity,
        });
    }

    const std::int64_t totalRegistrations = totalCurrent + totalCancelled;
    const int totalEvents = static_cast<int>(events.size());
    return model::DashboardStatistics{
        .comparisons = std::move(comparisons),
        .popularEvents = std::move(popularEvents),
        .statisticsByType = std::move(statisticsByType),
        .global = model::GlobalStatistics{
            .totalEvents = totalEvents,
            .fullEvents = fullEvents,
            .totalCapacity = totalCapacity,
            .totalCurrentParticipants = totalCurrent,
            .totalCancelledParticipants = totalCancelled,
            .totalAttendedParticipants = totalAttended,
            .averageParticipantsPerEvent = totalEvents == 0
                                                ? 0.0
                                                : static_cast<double>(totalCurrent) /
                                                      totalEvents,
            .overallOccupancyRate = percentage(totalCurrent, totalCapacity),
            .overallCancellationRate =
                percentage(totalCancelled, totalRegistrations),
            .overallAttendanceRate = percentage(totalAttended, totalCurrent),
        },
    };
}

}  // namespace campusconnect::statistics::service
