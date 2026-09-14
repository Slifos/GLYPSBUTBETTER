#include "grpc/StatisticsGrpcService.hpp"

#include <exception>

#include <spdlog/spdlog.h>

#include "exception/InvalidStatisticsException.hpp"

namespace campusconnect::statistics::transport {

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
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT, exception.what()};
    } catch (const std::exception& exception) {
        spdlog::error(
            "Unexpected error while calculating statistics for event {}: {}",
            eventId,
            exception.what());
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "An unexpected error occurred while calculating statistics"};
    }
}

}  // namespace campusconnect::statistics::transport
