#include <gtest/gtest.h>

#include "exception/InvalidStatisticsException.hpp"
#include "service/StatisticsService.hpp"

namespace {

using campusconnect::statistics::exception::InvalidStatisticsException;
using campusconnect::statistics::service::StatisticsService;

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

}  // namespace
