#pragma once

#include <grpcpp/grpcpp.h>

#include "service/StatisticsService.hpp"
#include "statistics.grpc.pb.h"

namespace campusconnect::statistics::transport {

class StatisticsGrpcService final
    : public campusconnect::statistics::StatisticsService::Service {
public:
    explicit StatisticsGrpcService(
        service::StatisticsService& statisticsService) noexcept;

    grpc::Status GetEventStatistics(
        grpc::ServerContext* context,
        const EventStatisticsRequest* request,
        EventStatisticsResponse* response) override;

private:
    service::StatisticsService& statisticsService_;
};

}  // namespace campusconnect::statistics::transport
