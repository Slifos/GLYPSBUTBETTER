#include "grpc/StatisticsGrpcService.hpp"

#include <exception>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "exception/InvalidStatisticsException.hpp"

namespace campusconnect::statistics::transport {

namespace {

// Converts generated protobuf input into a transport-independent business model.
model::EventData toModel(const EventData& event) {
    std::vector<model::RegistrationRecord> history;
    history.reserve(event.registration_history_size());
    for (const auto& record : event.registration_history()) {
        history.push_back(model::RegistrationRecord{
            .timestampEpochSeconds = record.timestamp_epoch_seconds(),
            .registrations = record.registrations(),
        });
    }

    return model::EventData{
        .eventId = event.event_id(),
        .eventType = event.event_type(),
        .maxParticipants = event.max_participants(),
        .currentParticipants = event.current_participants(),
        .cancelledParticipants = event.cancelled_participants(),
        .attendedParticipants = event.attended_participants(),
        .registrationHistory = std::move(history),
    };
}

// Converts the domain alert enum to its wire-format equivalent.
CapacityAlertLevel toProto(const model::CapacityAlertLevel level) {
    switch (level) {
        case model::CapacityAlertLevel::eightyPercent:
            return CAPACITY_ALERT_LEVEL_EIGHTY_PERCENT;
        case model::CapacityAlertLevel::ninetyPercent:
            return CAPACITY_ALERT_LEVEL_NINETY_PERCENT;
        case model::CapacityAlertLevel::full:
            return CAPACITY_ALERT_LEVEL_FULL;
    }
    return CAPACITY_ALERT_LEVEL_UNSPECIFIED;
}

// Preserves validation details for clients using the standard gRPC status code.
grpc::Status invalidArgument(const exception::InvalidStatisticsException& error) {
    return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, error.what()};
}

// Hides implementation details when an unexpected exception reaches the boundary.
grpc::Status internalError() {
    return grpc::Status{
        grpc::StatusCode::INTERNAL,
        "An unexpected error occurred while calculating statistics"};
}

}  // namespace

StatisticsGrpcService::StatisticsGrpcService(
    service::StatisticsService& statisticsService) noexcept
    : statisticsService_{statisticsService} {}

grpc::Status StatisticsGrpcService::GetEventStatistics(
    [[maybe_unused]] grpc::ServerContext* context,
    const EventStatisticsRequest* request,
    EventStatisticsResponse* response) {
    const auto eventId = request->event_id();
    spdlog::info("Statistics requested for event {}", eventId);

    try {
        const model::Statistics statistics = statisticsService_.calculate(
            request->max_participants(), request->current_participants());

        response->set_remaining_places(statistics.remainingPlaces);
        response->set_occupancy_rate(statistics.occupancyRate);
        response->set_full(statistics.full);

        spdlog::info(
            "Statistics calculated for event {}: occupancy={}%, remaining={}",
            eventId,
            statistics.occupancyRate,
            statistics.remainingPlaces);
        return grpc::Status::OK;
    } catch (const exception::InvalidStatisticsException& exception) {
        spdlog::warn(
            "Invalid statistics request for event {}: {}",
            eventId,
            exception.what());
        return invalidArgument(exception);
    } catch (const std::exception& exception) {
        spdlog::error(
            "Unexpected error while calculating statistics for event {}: {}",
            eventId,
            exception.what());
        return internalError();
    }
}

grpc::Status StatisticsGrpcService::AnalyzeEvent(
    [[maybe_unused]] grpc::ServerContext* context,
    const EventAnalyticsRequest* request,
    EventAnalyticsResponse* response) {
    const auto eventId = request->event().event_id();
    spdlog::info("Analytics requested for event {}", eventId);

    try {
        const model::EventAnalytics analytics = statisticsService_.analyzeEvent(
            toModel(request->event()), request->as_of_epoch_seconds());

        response->set_event_id(analytics.eventId);
        response->set_remaining_places(analytics.capacity.remainingPlaces);
        response->set_occupancy_rate(analytics.capacity.occupancyRate);
        response->set_full(analytics.capacity.full);
        response->set_cancellation_rate(analytics.cancellationRate);
        response->set_attended_participants(analytics.attendedParticipants);
        response->set_attendance_rate(analytics.attendanceRate);
        response->set_registrations_per_day(analytics.registrationsPerDay);
        if (analytics.estimatedFullAtEpochSeconds.has_value()) {
            response->set_estimated_full_at_epoch_seconds(
                *analytics.estimatedFullAtEpochSeconds);
        }

        for (const auto& bucket : analytics.dailyRegistrations) {
            auto* result = response->add_daily_registrations();
            result->set_period_start_epoch_seconds(
                bucket.periodStartEpochSeconds);
            result->set_registrations(bucket.registrations);
        }
        for (const auto& bucket : analytics.weeklyRegistrations) {
            auto* result = response->add_weekly_registrations();
            result->set_period_start_epoch_seconds(
                bucket.periodStartEpochSeconds);
            result->set_registrations(bucket.registrations);
        }
        for (const auto& alert : analytics.alerts) {
            auto* result = response->add_alerts();
            result->set_level(toProto(alert.level));
            result->set_threshold(alert.threshold);
            result->set_message(alert.message);
        }

        spdlog::info(
            "Analytics calculated for event {}: occupancy={}%, "
            "cancellations={}%, attendance={}%",
            eventId,
            analytics.capacity.occupancyRate,
            analytics.cancellationRate,
            analytics.attendanceRate);
        return grpc::Status::OK;
    } catch (const exception::InvalidStatisticsException& exception) {
        spdlog::warn(
            "Invalid analytics request for event {}: {}",
            eventId,
            exception.what());
        return invalidArgument(exception);
    } catch (const std::exception& exception) {
        spdlog::error(
            "Unexpected error while calculating analytics for event {}: {}",
            eventId,
            exception.what());
        return internalError();
    }
}

grpc::Status StatisticsGrpcService::GetDashboardStatistics(
    [[maybe_unused]] grpc::ServerContext* context,
    const DashboardStatisticsRequest* request,
    DashboardStatisticsResponse* response) {
    spdlog::info(
        "Dashboard statistics requested for {} events", request->events_size());

    try {
        if (request->popular_events_limit() < 0) {
            throw exception::InvalidStatisticsException{
                "popularEventsLimit cannot be negative"};
        }

        std::vector<model::EventData> events;
        events.reserve(request->events_size());
        for (const auto& event : request->events()) {
            events.push_back(toModel(event));
        }

        const model::DashboardStatistics dashboard =
            statisticsService_.calculateDashboard(
                events,
                request->as_of_epoch_seconds(),
                static_cast<std::size_t>(request->popular_events_limit()));

        for (const auto& comparison : dashboard.comparisons) {
            auto* result = response->add_comparisons();
            result->set_event_id(comparison.eventId);
            result->set_event_type(comparison.eventType);
            result->set_current_participants(comparison.currentParticipants);
            result->set_occupancy_rate(comparison.occupancyRate);
            result->set_cancellation_rate(comparison.cancellationRate);
            result->set_attendance_rate(comparison.attendanceRate);
            result->set_registrations_per_day(
                comparison.registrationsPerDay);
        }
        for (const auto& popularEvent : dashboard.popularEvents) {
            auto* result = response->add_popular_events();
            result->set_event_id(popularEvent.eventId);
            result->set_event_type(popularEvent.eventType);
            result->set_current_participants(
                popularEvent.currentParticipants);
            result->set_occupancy_rate(popularEvent.occupancyRate);
            result->set_rank(popularEvent.rank);
        }
        for (const auto& type : dashboard.statisticsByType) {
            auto* result = response->add_statistics_by_type();
            result->set_event_type(type.eventType);
            result->set_event_count(type.eventCount);
            result->set_average_participants(type.averageParticipants);
            result->set_average_attendance_rate(type.averageAttendanceRate);
            result->set_recommended_capacity(type.recommendedCapacity);
        }

        auto* global = response->mutable_global();
        global->set_total_events(dashboard.global.totalEvents);
        global->set_full_events(dashboard.global.fullEvents);
        global->set_total_capacity(dashboard.global.totalCapacity);
        global->set_total_current_participants(
            dashboard.global.totalCurrentParticipants);
        global->set_total_cancelled_participants(
            dashboard.global.totalCancelledParticipants);
        global->set_total_attended_participants(
            dashboard.global.totalAttendedParticipants);
        global->set_average_participants_per_event(
            dashboard.global.averageParticipantsPerEvent);
        global->set_overall_occupancy_rate(
            dashboard.global.overallOccupancyRate);
        global->set_overall_cancellation_rate(
            dashboard.global.overallCancellationRate);
        global->set_overall_attendance_rate(
            dashboard.global.overallAttendanceRate);

        spdlog::info(
            "Dashboard statistics calculated for {} events", events.size());
        return grpc::Status::OK;
    } catch (const exception::InvalidStatisticsException& exception) {
        spdlog::warn("Invalid dashboard request: {}", exception.what());
        return invalidArgument(exception);
    } catch (const std::exception& exception) {
        spdlog::error(
            "Unexpected error while calculating dashboard statistics: {}",
            exception.what());
        return internalError();
    }
}

}  // namespace campusconnect::statistics::transport
