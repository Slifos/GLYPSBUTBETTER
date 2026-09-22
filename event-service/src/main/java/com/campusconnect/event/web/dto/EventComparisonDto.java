package com.campusconnect.event.web.dto;

public record EventComparisonDto(
        Long eventId,
        String eventType,
        int currentParticipants,
        double occupancyRate,
        double cancellationRate,
        double attendanceRate,
        double registrationsPerDay
) {
}
