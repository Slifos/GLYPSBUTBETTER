package com.campusconnect.event.web.dto;

public record GlobalStatisticsDto(
        int totalEvents,
        int fullEvents,
        long totalCapacity,
        long totalCurrentParticipants,
        long totalCancelledParticipants,
        long totalAttendedParticipants,
        double averageParticipantsPerEvent,
        double overallOccupancyRate,
        double overallCancellationRate,
        double overallAttendanceRate
) {
}
