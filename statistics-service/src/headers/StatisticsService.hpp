#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Statistics.hpp"

class StatisticsService {
public:
    /**
     * Computes the current capacity indicators for one event.
     *
     * @param maxParticipants Maximum number of available places; must be positive.
     * @param currentParticipants Number of active registrations; cannot be negative.
     * @return Remaining places, occupancy percentage, and full status.
     * @throws InvalidStatisticsException when an input is invalid.
     */
    [[nodiscard]] Statistics calculate(
        int maxParticipants,
        int currentParticipants) const;

    /**
     * Produces detailed analytics from an event snapshot and registration history.
     *
     * Daily and weekly buckets use UTC boundaries. The filling forecast applies
     * the observed linear registration rate at the supplied analysis time.
     *
     * @param event Event counters and timestamped registration history.
     * @param asOfEpochSeconds Unix timestamp at which the snapshot is analysed.
     * @return Rates, time buckets, filling speed, forecast, and capacity alerts.
     * @throws InvalidStatisticsException when event data is inconsistent.
     */
    [[nodiscard]] EventAnalytics analyzeEvent(
        const EventData& event,
        std::int64_t asOfEpochSeconds) const;

    /**
     * Aggregates several event snapshots into administrator dashboard metrics.
     *
     * @param events Events to compare and aggregate; identifiers must be unique.
     * @param asOfEpochSeconds Unix timestamp used for every event analysis.
     * @param popularEventsLimit Maximum ranking size, or zero to return all events.
     * @return Comparisons, popularity ranking, per-type metrics, and global totals.
     * @throws InvalidStatisticsException when any event is invalid.
     */
    [[nodiscard]] DashboardStatistics calculateDashboard(
        const std::vector<EventData>& events,
        std::int64_t asOfEpochSeconds,
        std::size_t popularEventsLimit = 0) const;

private:
    struct EventTypeAccumulator {
        int eventCount{};
        std::int64_t currentParticipants{};
        std::int64_t attendedParticipants{};
        std::int64_t demand{};
    };

    static constexpr std::int64_t secondsPerDay = 86'400;

    /** Enforces the invariants shared by detailed and dashboard analyses. */
    static void validateEvent(
        const EventData& event,
        std::int64_t asOfEpochSeconds);

    /** Groups registrations into ordered UTC day or Monday-based week buckets. */
    [[nodiscard]] static std::vector<RegistrationBucket> aggregateRegistrations(
        const std::vector<RegistrationRecord>& history,
        bool weekly);

    /** Calculates a percentage and defines an empty denominator as zero. */
    [[nodiscard]] static double percentage(
        std::int64_t numerator,
        std::int64_t denominator);
};
