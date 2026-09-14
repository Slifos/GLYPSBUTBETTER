#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "exception/InvalidStatisticsException.hpp"
#include "model/Statistics.hpp"
#include "service/StatisticsService.hpp"

namespace {

using campusconnect::statistics::exception::InvalidStatisticsException;
using campusconnect::statistics::model::CapacityAlertLevel;
using campusconnect::statistics::model::EventData;
using campusconnect::statistics::model::RegistrationRecord;
using campusconnect::statistics::service::StatisticsService;

constexpr std::int64_t day = 86'400;

// Builds valid event fixtures while allowing each test to focus on one behaviour.
EventData createEvent(
    const std::int64_t id,
    std::string type,
    const int capacity,
    const int current,
    const int cancelled,
    const int attended) {
    return EventData{
        .eventId = id,
        .eventType = std::move(type),
        .maxParticipants = capacity,
        .currentParticipants = current,
        .cancelledParticipants = cancelled,
        .attendedParticipants = attended,
        .registrationHistory = {},
    };
}

TEST(StatisticsServiceTest, CalculatesStatisticsForPartiallyOccupiedEvent) {
    const StatisticsService service;

    const auto statistics = service.calculate(10, 7);

    EXPECT_EQ(statistics.remainingPlaces, 3);
    EXPECT_DOUBLE_EQ(statistics.occupancyRate, 70.0);
    EXPECT_FALSE(statistics.full);
}

TEST(StatisticsServiceTest, CalculatesStatisticsForFullEvent) {
    const StatisticsService service;

    const auto statistics = service.calculate(10, 10);

    EXPECT_EQ(statistics.remainingPlaces, 0);
    EXPECT_DOUBLE_EQ(statistics.occupancyRate, 100.0);
    EXPECT_TRUE(statistics.full);
}

TEST(StatisticsServiceTest, DoesNotCapOccupancyForOverbookedEvent) {
    const StatisticsService service;

    const auto statistics = service.calculate(10, 12);

    EXPECT_EQ(statistics.remainingPlaces, 0);
    EXPECT_DOUBLE_EQ(statistics.occupancyRate, 120.0);
    EXPECT_TRUE(statistics.full);
}

TEST(StatisticsServiceTest, CalculatesStatisticsForEmptyEvent) {
    const StatisticsService service;

    const auto statistics = service.calculate(1, 0);

    EXPECT_EQ(statistics.remainingPlaces, 1);
    EXPECT_DOUBLE_EQ(statistics.occupancyRate, 0.0);
    EXPECT_FALSE(statistics.full);
}

TEST(StatisticsServiceTest, RejectsZeroMaximumParticipants) {
    const StatisticsService service;

    EXPECT_THROW(
        static_cast<void>(service.calculate(0, 0)), InvalidStatisticsException);
}

TEST(StatisticsServiceTest, RejectsNegativeMaximumParticipants) {
    const StatisticsService service;

    EXPECT_THROW(
        static_cast<void>(service.calculate(-1, 0)), InvalidStatisticsException);
}

TEST(StatisticsServiceTest, RejectsNegativeCurrentParticipants) {
    const StatisticsService service;

    EXPECT_THROW(
        static_cast<void>(service.calculate(10, -1)), InvalidStatisticsException);
}

TEST(StatisticsServiceTest, InvalidInputHasAUsefulErrorMessage) {
    const StatisticsService service;

    try {
        static_cast<void>(service.calculate(0, 0));
        FAIL() << "Expected InvalidStatisticsException";
    } catch (const InvalidStatisticsException& exception) {
        EXPECT_STREQ(exception.what(), "maxParticipants must be greater than 0");
    }
}

TEST(StatisticsServiceTest, CalculatesDetailedEventAnalytics) {
    const StatisticsService service;
    EventData event = createEvent(42, "workshop", 10, 5, 5, 4);
    event.registrationHistory = {
        RegistrationRecord{.timestampEpochSeconds = 8 * day, .registrations = 2},
        RegistrationRecord{.timestampEpochSeconds = 9 * day, .registrations = 3},
    };

    const auto analytics = service.analyzeEvent(event, 10 * day);

    EXPECT_EQ(analytics.eventId, 42);
    EXPECT_EQ(analytics.capacity.remainingPlaces, 5);
    EXPECT_DOUBLE_EQ(analytics.capacity.occupancyRate, 50.0);
    EXPECT_DOUBLE_EQ(analytics.cancellationRate, 50.0);
    EXPECT_EQ(analytics.attendedParticipants, 4);
    EXPECT_DOUBLE_EQ(analytics.attendanceRate, 80.0);
    EXPECT_DOUBLE_EQ(analytics.registrationsPerDay, 2.5);
    ASSERT_TRUE(analytics.estimatedFullAtEpochSeconds.has_value());
    EXPECT_EQ(*analytics.estimatedFullAtEpochSeconds, 12 * day);

    ASSERT_EQ(analytics.dailyRegistrations.size(), 2);
    EXPECT_EQ(analytics.dailyRegistrations[0].periodStartEpochSeconds, 8 * day);
    EXPECT_EQ(analytics.dailyRegistrations[0].registrations, 2);
    EXPECT_EQ(analytics.dailyRegistrations[1].periodStartEpochSeconds, 9 * day);
    EXPECT_EQ(analytics.dailyRegistrations[1].registrations, 3);

    ASSERT_EQ(analytics.weeklyRegistrations.size(), 1);
    EXPECT_EQ(analytics.weeklyRegistrations[0].periodStartEpochSeconds, 4 * day);
    EXPECT_EQ(analytics.weeklyRegistrations[0].registrations, 5);
    EXPECT_TRUE(analytics.alerts.empty());
}

TEST(StatisticsServiceTest, EmitsEveryReachedCapacityAlert) {
    const StatisticsService service;
    const EventData event = createEvent(42, "concert", 10, 10, 0, 8);

    const auto analytics = service.analyzeEvent(event, 10 * day);

    ASSERT_EQ(analytics.alerts.size(), 3);
    EXPECT_EQ(analytics.alerts[0].level, CapacityAlertLevel::eightyPercent);
    EXPECT_EQ(analytics.alerts[1].level, CapacityAlertLevel::ninetyPercent);
    EXPECT_EQ(analytics.alerts[2].level, CapacityAlertLevel::full);
    ASSERT_TRUE(analytics.estimatedFullAtEpochSeconds.has_value());
    EXPECT_EQ(*analytics.estimatedFullAtEpochSeconds, 10 * day);
}

TEST(StatisticsServiceTest, CannotForecastWithoutRegistrationHistory) {
    const StatisticsService service;
    const EventData event = createEvent(42, "conference", 100, 20, 0, 0);

    const auto analytics = service.analyzeEvent(event, 10 * day);

    EXPECT_DOUBLE_EQ(analytics.registrationsPerDay, 0.0);
    EXPECT_FALSE(analytics.estimatedFullAtEpochSeconds.has_value());
}

TEST(StatisticsServiceTest, RejectsInconsistentAttendance) {
    const StatisticsService service;
    const EventData event = createEvent(42, "conference", 100, 20, 0, 21);

    EXPECT_THROW(
        static_cast<void>(service.analyzeEvent(event, 10 * day)),
        InvalidStatisticsException);
}

TEST(StatisticsServiceTest, RejectsRegistrationAfterAnalysisTime) {
    const StatisticsService service;
    EventData event = createEvent(42, "conference", 100, 20, 0, 10);
    event.registrationHistory = {
        RegistrationRecord{
            .timestampEpochSeconds = 11 * day,
            .registrations = 1,
        },
    };

    EXPECT_THROW(
        static_cast<void>(service.analyzeEvent(event, 10 * day)),
        InvalidStatisticsException);
}

TEST(StatisticsServiceTest, CalculatesDashboardStatisticsAndPopularity) {
    const StatisticsService service;
    const std::vector<EventData> events{
        createEvent(1, "workshop", 10, 8, 2, 6),
        createEvent(2, "workshop", 20, 12, 0, 9),
        createEvent(3, "concert", 10, 10, 5, 8),
    };

    const auto dashboard = service.calculateDashboard(events, 10 * day, 2);

    ASSERT_EQ(dashboard.comparisons.size(), 3);
    EXPECT_EQ(dashboard.comparisons[0].eventId, 1);
    EXPECT_DOUBLE_EQ(dashboard.comparisons[0].occupancyRate, 80.0);

    ASSERT_EQ(dashboard.popularEvents.size(), 2);
    EXPECT_EQ(dashboard.popularEvents[0].eventId, 2);
    EXPECT_EQ(dashboard.popularEvents[0].rank, 1);
    EXPECT_EQ(dashboard.popularEvents[1].eventId, 3);
    EXPECT_EQ(dashboard.popularEvents[1].rank, 2);

    ASSERT_EQ(dashboard.statisticsByType.size(), 2);
    EXPECT_EQ(dashboard.statisticsByType[0].eventType, "concert");
    EXPECT_EQ(dashboard.statisticsByType[0].eventCount, 1);
    EXPECT_DOUBLE_EQ(dashboard.statisticsByType[0].averageParticipants, 10.0);
    EXPECT_DOUBLE_EQ(dashboard.statisticsByType[0].averageAttendanceRate, 80.0);
    EXPECT_EQ(dashboard.statisticsByType[0].recommendedCapacity, 17);
    EXPECT_EQ(dashboard.statisticsByType[1].eventType, "workshop");
    EXPECT_DOUBLE_EQ(dashboard.statisticsByType[1].averageParticipants, 10.0);
    EXPECT_DOUBLE_EQ(dashboard.statisticsByType[1].averageAttendanceRate, 75.0);
    EXPECT_EQ(dashboard.statisticsByType[1].recommendedCapacity, 13);

    EXPECT_EQ(dashboard.global.totalEvents, 3);
    EXPECT_EQ(dashboard.global.fullEvents, 1);
    EXPECT_EQ(dashboard.global.totalCapacity, 40);
    EXPECT_EQ(dashboard.global.totalCurrentParticipants, 30);
    EXPECT_EQ(dashboard.global.totalCancelledParticipants, 7);
    EXPECT_EQ(dashboard.global.totalAttendedParticipants, 23);
    EXPECT_DOUBLE_EQ(dashboard.global.averageParticipantsPerEvent, 10.0);
    EXPECT_DOUBLE_EQ(dashboard.global.overallOccupancyRate, 75.0);
    EXPECT_NEAR(dashboard.global.overallCancellationRate, 18.9189, 0.0001);
    EXPECT_NEAR(dashboard.global.overallAttendanceRate, 76.6667, 0.0001);
}

TEST(StatisticsServiceTest, ReturnsAnEmptyDashboardForNoEvents) {
    const StatisticsService service;

    const auto dashboard = service.calculateDashboard({}, 10 * day);

    EXPECT_TRUE(dashboard.comparisons.empty());
    EXPECT_TRUE(dashboard.popularEvents.empty());
    EXPECT_TRUE(dashboard.statisticsByType.empty());
    EXPECT_EQ(dashboard.global.totalEvents, 0);
    EXPECT_DOUBLE_EQ(dashboard.global.overallOccupancyRate, 0.0);
}

TEST(StatisticsServiceTest, RejectsDuplicateEventIdsInDashboard) {
    const StatisticsService service;
    const std::vector<EventData> events{
        createEvent(1, "workshop", 10, 8, 2, 6),
        createEvent(1, "concert", 20, 12, 0, 9),
    };

    EXPECT_THROW(
        static_cast<void>(service.calculateDashboard(events, 10 * day)),
        InvalidStatisticsException);
}

}  // namespace
