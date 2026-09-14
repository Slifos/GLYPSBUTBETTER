#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "model/Statistics.hpp"

namespace campusconnect::statistics::service {

class StatisticsService {
public:
    /**
     * Computes the current capacity indicators for one event.
     *
     * @param maxParticipants Maximum number of available places; must be positive.
     * @param currentParticipants Number of active registrations; cannot be negative.
     * @return Remaining places, occupancy percentage, and full status.
     * @throws exception::InvalidStatisticsException when an input is invalid.
     */
    [[nodiscard]] model::Statistics calculate(
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
     * @throws exception::InvalidStatisticsException when event data is inconsistent.
     */
    [[nodiscard]] model::EventAnalytics analyzeEvent(
        const model::EventData& event,
        std::int64_t asOfEpochSeconds) const;

    /**
     * Aggregates several event snapshots into administrator dashboard metrics.
     *
     * @param events Events to compare and aggregate; identifiers must be unique.
     * @param asOfEpochSeconds Unix timestamp used for every event analysis.
     * @param popularEventsLimit Maximum ranking size, or zero to return all events.
     * @return Comparisons, popularity ranking, per-type metrics, and global totals.
     * @throws exception::InvalidStatisticsException when any event is invalid.
     */
    [[nodiscard]] model::DashboardStatistics calculateDashboard(
        const std::vector<model::EventData>& events,
        std::int64_t asOfEpochSeconds,
        std::size_t popularEventsLimit = 0) const;
};

}  // namespace campusconnect::statistics::service
