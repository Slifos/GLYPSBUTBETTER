#pragma once

#include <grpcpp/grpcpp.h>

#include "InvalidStatisticsException.hpp"
#include "StatisticsService.hpp"
#include "statistics.grpc.pb.h"

class StatisticsGrpcService final
    : public campusconnect::statistics::StatisticsService::Service {
public:
    /**
     * Creates the transport adapter around the stateless business service.
     *
     * @param statisticsService Business service that must outlive this adapter.
     */
    explicit StatisticsGrpcService(StatisticsService& statisticsService) noexcept;

    /** Maps the legacy capacity request to the business service and protobuf response. */
    grpc::Status GetEventStatistics(
        grpc::ServerContext* context,
        const campusconnect::statistics::EventStatisticsRequest* request,
        campusconnect::statistics::EventStatisticsResponse* response) override;

    /** Maps a detailed single-event analysis request to its protobuf response. */
    grpc::Status AnalyzeEvent(
        grpc::ServerContext* context,
        const campusconnect::statistics::EventAnalyticsRequest* request,
        campusconnect::statistics::EventAnalyticsResponse* response) override;

    /** Maps a multi-event request to comparison and dashboard protobuf messages. */
    grpc::Status GetDashboardStatistics(
        grpc::ServerContext* context,
        const campusconnect::statistics::DashboardStatisticsRequest* request,
        campusconnect::statistics::DashboardStatisticsResponse* response) override;

private:
    /** Converts a generated protobuf event into the business model. */
    [[nodiscard]] static EventData toModel(
        const campusconnect::statistics::EventData& event);

    /** Converts a domain capacity alert level into its protobuf enum. */
    [[nodiscard]] static campusconnect::statistics::CapacityAlertLevel toProto(
        CapacityAlertLevel level);

    /** Builds an INVALID_ARGUMENT status while retaining the validation message. */
    [[nodiscard]] static grpc::Status invalidArgument(
        const InvalidStatisticsException& error);

    /** Builds a generic INTERNAL status that does not expose implementation details. */
    [[nodiscard]] static grpc::Status internalError();

    StatisticsService& statisticsService_;
};
