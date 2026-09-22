package com.campusconnect.event.web.dto;

public record PopularEventDto(
        Long eventId,
        String eventType,
        int currentParticipants,
        double occupancyRate,
        int rank
) {
}
