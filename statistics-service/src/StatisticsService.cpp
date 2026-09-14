#include "StatisticsService.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include "InvalidStatisticsException.hpp"

void StatisticsService::validateEvent(
    const EventData& event,
    const std::int64_t asOfEpochSeconds) {
    if (asOfEpochSeconds <= 0) {
        throw InvalidStatisticsException{
            "asOfEpochSeconds must be greater than 0"};
    }
    if (event.eventId <= 0) {
        throw InvalidStatisticsException{
            "eventId must be greater than 0"};
    }
    if (event.eventType.empty()) {
        throw InvalidStatisticsException{
            "eventType cannot be empty"};
    }
    if (event.maxParticipants <= 0) {
        throw InvalidStatisticsException{
            "maxParticipants must be greater than 0"};
    }
    if (event.currentParticipants < 0) {
        throw InvalidStatisticsException{
            "currentParticipants cannot be negative"};
    }
    if (event.cancelledParticipants < 0) {
        throw InvalidStatisticsException{
            "cancelledParticipants cannot be negative"};
    }
    if (event.attendedParticipants < 0) {
        throw InvalidStatisticsException{
            "attendedParticipants cannot be negative"};
    }
    if (event.attendedParticipants > event.currentParticipants) {
        throw InvalidStatisticsException{
            "attendedParticipants cannot exceed currentParticipants"};
    }

    for (const auto& record : event.registrationHistory) {
        if (record.timestampEpochSeconds < 0) {
            throw InvalidStatisticsException{
                "registration timestamp cannot be negative"};
        }
        if (record.timestampEpochSeconds > asOfEpochSeconds) {
            throw InvalidStatisticsException{
                "registration timestamp cannot be after asOfEpochSeconds"};
        }
        if (record.registrations <= 0) {
            throw InvalidStatisticsException{
                "registration count must be greater than 0"};
        }
    }
}

std::vector<RegistrationBucket> StatisticsService::aggregateRegistrations(
    const std::vector<RegistrationRecord>& history,
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

    std::vector<RegistrationBucket> buckets;
    buckets.reserve(registrationsByPeriod.size());
    for (const auto& [periodStart, registrations] : registrationsByPeriod) {
        if (registrations > std::numeric_limits<int>::max()) {
            throw InvalidStatisticsException{
                "registration bucket exceeds the supported range"};
        }
        buckets.push_back(RegistrationBucket{
            .periodStartEpochSeconds = periodStart,
            .registrations = static_cast<int>(registrations),
        });
    }
    return buckets;
}

double StatisticsService::percentage(
    const std::int64_t numerator,
    const std::int64_t denominator) {
    if (denominator == 0) {
        return 0.0;
    }
    return static_cast<double>(numerator) /
           static_cast<double>(denominator) * 100.0;
}

Statistics StatisticsService::calculate(
    const int maxParticipants,
    const int currentParticipants) const {
    if (maxParticipants <= 0) {
        throw InvalidStatisticsException{
            "maxParticipants must be greater than 0"};
    }

    if (currentParticipants < 0) {
        throw InvalidStatisticsException{
            "currentParticipants cannot be negative"};
    }

    const int remainingPlaces = currentParticipants >= maxParticipants
                                    ? 0
                                    : maxParticipants - currentParticipants;
    const double occupancyRate =
        static_cast<double>(currentParticipants) /
        static_cast<double>(maxParticipants) * 100.0;

    return Statistics{
        .remainingPlaces = remainingPlaces,
        .occupancyRate = occupancyRate,
        .full = currentParticipants >= maxParticipants,
    };
}

EventAnalytics StatisticsService::analyzeEvent(
    const EventData& event,
    const std::int64_t asOfEpochSeconds) const {
    validateEvent(event, asOfEpochSeconds);

    const Statistics capacity =
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
            throw InvalidStatisticsException{
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

    std::vector<CapacityAlert> alerts;
    if (capacity.occupancyRate >= 80.0) {
        alerts.push_back(CapacityAlert{
            .level = CapacityAlertLevel::eightyPercent,
            .threshold = 80.0,
            .message = "Event capacity has reached 80%",
        });
    }
    if (capacity.occupancyRate >= 90.0) {
        alerts.push_back(CapacityAlert{
            .level = CapacityAlertLevel::ninetyPercent,
            .threshold = 90.0,
            .message = "Event capacity has reached 90%",
        });
    }
    if (capacity.full) {
        alerts.push_back(CapacityAlert{
            .level = CapacityAlertLevel::full,
            .threshold = 100.0,
            .message = "Event is full",
        });
    }

    return EventAnalytics{
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

DashboardStatistics StatisticsService::calculateDashboard(
    const std::vector<EventData>& events,
    const std::int64_t asOfEpochSeconds,
    const std::size_t popularEventsLimit) const {
    if (asOfEpochSeconds <= 0) {
        throw InvalidStatisticsException{
            "asOfEpochSeconds must be greater than 0"};
    }

    std::set<std::int64_t> eventIds;
    std::vector<EventComparison> comparisons;
    std::vector<PopularEvent> popularEvents;
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
            throw InvalidStatisticsException{
                "eventId values must be unique"};
        }

        const EventAnalytics analytics =
            analyzeEvent(event, asOfEpochSeconds);
        comparisons.push_back(EventComparison{
            .eventId = event.eventId,
            .eventType = event.eventType,
            .currentParticipants = event.currentParticipants,
            .occupancyRate = analytics.capacity.occupancyRate,
            .cancellationRate = analytics.cancellationRate,
            .attendanceRate = analytics.attendanceRate,
            .registrationsPerDay = analytics.registrationsPerDay,
        });
        popularEvents.push_back(PopularEvent{
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
        [](const PopularEvent& left, const PopularEvent& right) {
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

    std::vector<EventTypeStatistics> statisticsByType;
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

        statisticsByType.push_back(EventTypeStatistics{
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
    return DashboardStatistics{
        .comparisons = std::move(comparisons),
        .popularEvents = std::move(popularEvents),
        .statisticsByType = std::move(statisticsByType),
        .global = GlobalStatistics{
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
