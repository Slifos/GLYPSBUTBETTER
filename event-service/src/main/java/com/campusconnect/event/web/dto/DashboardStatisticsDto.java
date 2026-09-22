package com.campusconnect.event.web.dto;

import java.util.List;

/** Java-side view of statistics-service's {@code DashboardStatisticsResponse}. */
public record DashboardStatisticsDto(
        List<EventComparisonDto> comparisons,
        List<PopularEventDto> popularEvents,
        List<EventTypeStatisticsDto> statisticsByType,
        GlobalStatisticsDto global
) {
}
