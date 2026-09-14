#pragma once

#include <grpcpp/grpcpp.h>

#include "service/StatisticsService.hpp"
#include "statistics.grpc.pb.h"

namespace campusconnect::statistics::transport {

class StatisticsGrpcService final
    : public campusconnect::statistics::StatisticsService::Service {
public:
    /**
     * Creates the transport adapter around the stateless business service.
     *
     * @param statisticsService Business service that must outlive this adapter.
     */
    explicit StatisticsGrpcService(
        service::StatisticsService& statisticsService) noexcept;

    /** Maps the legacy capacity request to the business service and protobuf response. */
    grpc::Status GetEventStatistics(
        grpc::ServerContext* context,
        const EventStatisticsRequest* request,
        EventStatisticsResponse* response) override;

    /** Maps a detailed single-event analysis request to its protobuf response. */
    grpc::Status AnalyzeEvent(
        grpc::ServerContext* context,
        const EventAnalyticsRequest* request,
        EventAnalyticsResponse* response) override;

    /** Maps a multi-event request to comparison and dashboard protobuf messages. */
    grpc::Status GetDashboardStatistics(
        grpc::ServerContext* context,
        const DashboardStatisticsRequest* request,
        DashboardStatisticsResponse* response) override;

private:
    service::StatisticsService& statisticsService_;
};

}  // namespace campusconnect::statistics::transport
