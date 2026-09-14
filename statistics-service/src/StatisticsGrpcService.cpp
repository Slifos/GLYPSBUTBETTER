#include "StatisticsGrpcService.hpp"

#include <exception>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "InvalidStatisticsException.hpp"

EventData StatisticsGrpcService::toModel(
    const campusconnect::statistics::EventData& event) {
    std::vector<RegistrationRecord> history;
    history.reserve(event.registration_history_size());
    for (const auto& record : event.registration_history()) {
        history.push_back(RegistrationRecord{
            .timestampEpochSeconds = record.timestamp_epoch_seconds(),
            .registrations = record.registrations(),
        });
    }

    return EventData{
        .eventId = event.event_id(),
        .eventType = event.event_type(),
        .maxParticipants = event.max_participants(),
        .currentParticipants = event.current_participants(),
        .cancelledParticipants = event.cancelled_participants(),
        .attendedParticipants = event.attended_participants(),
        .registrationHistory = std::move(history),
    };
}

campusconnect::statistics::CapacityAlertLevel StatisticsGrpcService::toProto(
    const CapacityAlertLevel level) {
    switch (level) {
        case CapacityAlertLevel::eightyPercent:
            return campusconnect::statistics::CAPACITY_ALERT_LEVEL_EIGHTY_PERCENT;
        case CapacityAlertLevel::ninetyPercent:
            return campusconnect::statistics::CAPACITY_ALERT_LEVEL_NINETY_PERCENT;
        case CapacityAlertLevel::full:
            return campusconnect::statistics::CAPACITY_ALERT_LEVEL_FULL;
    }
    return campusconnect::statistics::CAPACITY_ALERT_LEVEL_UNSPECIFIED;
}

grpc::Status StatisticsGrpcService::invalidArgument(
    const InvalidStatisticsException& error) {
    return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, error.what()};
}

grpc::Status StatisticsGrpcService::internalError() {
    return grpc::Status{
        grpc::StatusCode::INTERNAL,
        "An unexpected error occurred while calculating statistics"};
}

StatisticsGrpcService::StatisticsGrpcService(
    StatisticsService& statisticsService) noexcept
    : statisticsService_{statisticsService} {}

grpc::Status StatisticsGrpcService::GetEventStatistics(
    [[maybe_unused]] grpc::ServerContext* context,
    const campusconnect::statistics::EventStatisticsRequest* request,
    campusconnect::statistics::EventStatisticsResponse* response) {
    const auto eventId = request->event_id();
    spdlog::info("Statistics requested for event {}", eventId);

    try {
        const Statistics statistics = statisticsService_.calculate(
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
    } catch (const InvalidStatisticsException& exception) {
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
    const campusconnect::statistics::EventAnalyticsRequest* request,
    campusconnect::statistics::EventAnalyticsResponse* response) {
    const auto eventId = request->event().event_id();
    spdlog::info("Analytics requested for event {}", eventId);

    try {
        const EventAnalytics analytics = statisticsService_.analyzeEvent(
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
    } catch (const InvalidStatisticsException& exception) {
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
    const campusconnect::statistics::DashboardStatisticsRequest* request,
    campusconnect::statistics::DashboardStatisticsResponse* response) {
    spdlog::info(
        "Dashboard statistics requested for {} events", request->events_size());

    try {
        if (request->popular_events_limit() < 0) {
            throw InvalidStatisticsException{
                "popularEventsLimit cannot be negative"};
        }

        std::vector<EventData> events;
        events.reserve(request->events_size());
        for (const auto& event : request->events()) {
            events.push_back(toModel(event));
        }

        const DashboardStatistics dashboard =
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
    } catch (const InvalidStatisticsException& exception) {
        spdlog::warn("Invalid dashboard request: {}", exception.what());
        return invalidArgument(exception);
    } catch (const std::exception& exception) {
        spdlog::error(
            "Unexpected error while calculating dashboard statistics: {}",
            exception.what());
        return internalError();
    }
}
