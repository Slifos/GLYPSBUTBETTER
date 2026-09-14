#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace campusconnect::statistics::model {

struct Statistics {
    int remainingPlaces;
    double occupancyRate;
    bool full;
};

struct RegistrationRecord {
    std::int64_t timestampEpochSeconds;
    int registrations;
};

struct EventData {
    std::int64_t eventId;
    std::string eventType;
    int maxParticipants;
    int currentParticipants;
    int cancelledParticipants;
    int attendedParticipants;
    std::vector<RegistrationRecord> registrationHistory;
};

struct RegistrationBucket {
    std::int64_t periodStartEpochSeconds;
    int registrations;
};

enum class CapacityAlertLevel {
    eightyPercent,
    ninetyPercent,
    full,
};

struct CapacityAlert {
    CapacityAlertLevel level;
    double threshold;
    std::string message;
};

struct EventAnalytics {
    std::int64_t eventId;
    Statistics capacity;
    double cancellationRate;
    int attendedParticipants;
    double attendanceRate;
    std::vector<RegistrationBucket> dailyRegistrations;
    std::vector<RegistrationBucket> weeklyRegistrations;
    double registrationsPerDay;
    std::optional<std::int64_t> estimatedFullAtEpochSeconds;
    std::vector<CapacityAlert> alerts;
};

struct EventComparison {
    std::int64_t eventId;
    std::string eventType;
    int currentParticipants;
    double occupancyRate;
    double cancellationRate;
    double attendanceRate;
    double registrationsPerDay;
};

struct PopularEvent {
    std::int64_t eventId;
    std::string eventType;
    int currentParticipants;
    double occupancyRate;
    int rank;
};

struct EventTypeStatistics {
    std::string eventType;
    int eventCount;
    double averageParticipants;
    double averageAttendanceRate;
    int recommendedCapacity;
};

struct GlobalStatistics {
    int totalEvents;
    int fullEvents;
    std::int64_t totalCapacity;
    std::int64_t totalCurrentParticipants;
    std::int64_t totalCancelledParticipants;
    std::int64_t totalAttendedParticipants;
    double averageParticipantsPerEvent;
    double overallOccupancyRate;
    double overallCancellationRate;
    double overallAttendanceRate;
};

struct DashboardStatistics {
    std::vector<EventComparison> comparisons;
    std::vector<PopularEvent> popularEvents;
    std::vector<EventTypeStatistics> statisticsByType;
    GlobalStatistics global;
};

}  // namespace campusconnect::statistics::model
