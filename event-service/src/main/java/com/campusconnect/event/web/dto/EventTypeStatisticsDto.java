package com.campusconnect.event.web.dto;

public record EventTypeStatisticsDto(
        String eventType,
        int eventCount,
        double averageParticipants,
        double averageAttendanceRate,
        int recommendedCapacity
) {
}
