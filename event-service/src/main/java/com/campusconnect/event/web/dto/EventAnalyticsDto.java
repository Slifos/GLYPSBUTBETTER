package com.campusconnect.event.web.dto;

import java.time.Instant;
import java.util.List;

/** Java-side view of statistics-service's {@code EventAnalyticsResponse}. */
public record EventAnalyticsDto(
        Long eventId,
        int remainingPlaces,
        double occupancyRate,
        boolean full,
        double cancellationRate,
        int attendedParticipants,
        double attendanceRate,
        List<RegistrationBucketDto> dailyRegistrations,
        List<RegistrationBucketDto> weeklyRegistrations,
        double registrationsPerDay,
        Instant estimatedFullAt,
        List<CapacityAlertDto> alerts
) {
}
