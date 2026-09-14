#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include <grpcpp/grpcpp.h>

#include "StatisticsGrpcService.hpp"
#include "StatisticsService.hpp"
#include "statistics.pb.h"

using campusconnect::statistics::EventStatisticsRequest;
using campusconnect::statistics::EventStatisticsResponse;
using campusconnect::statistics::EventAnalyticsRequest;
using campusconnect::statistics::EventAnalyticsResponse;
using campusconnect::statistics::DashboardStatisticsRequest;
using campusconnect::statistics::DashboardStatisticsResponse;

constexpr std::int64_t day = 86'400;

// Populates the common protobuf fields used by transport-level test requests.
void populateEvent(
    campusconnect::statistics::EventData* event,
    const std::int64_t id,
    const std::string& type,
    const int capacity,
    const int current,
    const int cancelled,
    const int attended) {
    event->set_event_id(id);
    event->set_event_type(type);
    event->set_max_participants(capacity);
    event->set_current_participants(current);
    event->set_cancelled_participants(cancelled);
    event->set_attended_participants(attended);
}

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

TEST(StatisticsGrpcServiceTest, MapsEventAnalyticsToProtobufResponse) {
    StatisticsService statisticsService;
    StatisticsGrpcService grpcService{statisticsService};
    grpc::ServerContext context;
    EventAnalyticsRequest request;
    EventAnalyticsResponse response;
    request.set_as_of_epoch_seconds(10 * day);
    populateEvent(request.mutable_event(), 42, "concert", 10, 9, 1, 8);
    auto* first = request.mutable_event()->add_registration_history();
    first->set_timestamp_epoch_seconds(8 * day);
    first->set_registrations(4);
    auto* second = request.mutable_event()->add_registration_history();
    second->set_timestamp_epoch_seconds(9 * day);
    second->set_registrations(5);

    const grpc::Status status =
        grpcService.AnalyzeEvent(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.event_id(), 42);
    EXPECT_EQ(response.remaining_places(), 1);
    EXPECT_DOUBLE_EQ(response.occupancy_rate(), 90.0);
    EXPECT_DOUBLE_EQ(response.cancellation_rate(), 10.0);
    EXPECT_DOUBLE_EQ(response.attendance_rate(), 800.0 / 9.0);
    EXPECT_DOUBLE_EQ(response.registrations_per_day(), 4.5);
    EXPECT_GT(response.estimated_full_at_epoch_seconds(), 10 * day);
    EXPECT_EQ(response.daily_registrations_size(), 2);
    EXPECT_EQ(response.weekly_registrations_size(), 1);
    ASSERT_EQ(response.alerts_size(), 2);
    EXPECT_EQ(
        response.alerts(0).level(),
        campusconnect::statistics::CAPACITY_ALERT_LEVEL_EIGHTY_PERCENT);
    EXPECT_EQ(
        response.alerts(1).level(),
        campusconnect::statistics::CAPACITY_ALERT_LEVEL_NINETY_PERCENT);
}

TEST(StatisticsGrpcServiceTest, MapsDashboardToProtobufResponse) {
    StatisticsService statisticsService;
    StatisticsGrpcService grpcService{statisticsService};
    grpc::ServerContext context;
    DashboardStatisticsRequest request;
    DashboardStatisticsResponse response;
    request.set_as_of_epoch_seconds(10 * day);
    request.set_popular_events_limit(1);
    populateEvent(request.add_events(), 1, "workshop", 10, 8, 2, 6);
    populateEvent(request.add_events(), 2, "concert", 20, 12, 0, 9);

    const grpc::Status status = grpcService.GetDashboardStatistics(
        &context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.comparisons_size(), 2);
    ASSERT_EQ(response.popular_events_size(), 1);
    EXPECT_EQ(response.popular_events(0).event_id(), 2);
    EXPECT_EQ(response.popular_events(0).rank(), 1);
    EXPECT_EQ(response.statistics_by_type_size(), 2);
    EXPECT_EQ(response.global().total_events(), 2);
    EXPECT_EQ(response.global().total_current_participants(), 20);
}

TEST(StatisticsGrpcServiceTest, RejectsNegativePopularityLimit) {
    StatisticsService statisticsService;
    StatisticsGrpcService grpcService{statisticsService};
    grpc::ServerContext context;
    DashboardStatisticsRequest request;
    DashboardStatisticsResponse response;
    request.set_as_of_epoch_seconds(10 * day);
    request.set_popular_events_limit(-1);

    const grpc::Status status = grpcService.GetDashboardStatistics(
        &context, &request, &response);

    EXPECT_EQ(status.error_code(), grpc::StatusCode::INVALID_ARGUMENT);
    EXPECT_EQ(status.error_message(), "popularEventsLimit cannot be negative");
}
