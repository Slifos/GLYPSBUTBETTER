#include <gtest/gtest.h>

#include <grpcpp/grpcpp.h>

#include "grpc/StatisticsGrpcService.hpp"
#include "service/StatisticsService.hpp"
#include "statistics.pb.h"

namespace {

using campusconnect::statistics::EventStatisticsRequest;
using campusconnect::statistics::EventStatisticsResponse;
using campusconnect::statistics::service::StatisticsService;
using campusconnect::statistics::transport::StatisticsGrpcService;

TEST(StatisticsGrpcServiceTest, MapsAValidRequestToTheProtobufResponse) {
    StatisticsService statisticsService;
    StatisticsGrpcService grpcService{statisticsService};
    grpc::ServerContext context;
    EventStatisticsRequest request;
    EventStatisticsResponse response;
    request.set_event_id(42);
    request.set_max_participants(10);
    request.set_current_participants(7);

    const grpc::Status status =
        grpcService.GetEventStatistics(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.remaining_places(), 3);
    EXPECT_DOUBLE_EQ(response.occupancy_rate(), 70.0);
    EXPECT_FALSE(response.full());
}

TEST(StatisticsGrpcServiceTest, MapsInvalidInputToInvalidArgument) {
    StatisticsService statisticsService;
    StatisticsGrpcService grpcService{statisticsService};
    grpc::ServerContext context;
    EventStatisticsRequest request;
    EventStatisticsResponse response;
    request.set_event_id(42);
    request.set_max_participants(0);
    request.set_current_participants(0);

    const grpc::Status status =
        grpcService.GetEventStatistics(&context, &request, &response);

    EXPECT_EQ(status.error_code(), grpc::StatusCode::INVALID_ARGUMENT);
    EXPECT_EQ(status.error_message(), "maxParticipants must be greater than 0");
}

}  // namespace
