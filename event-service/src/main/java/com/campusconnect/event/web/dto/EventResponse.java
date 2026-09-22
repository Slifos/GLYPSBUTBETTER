package com.campusconnect.event.web.dto;

import java.time.LocalDateTime;

public record EventResponse(
        Long id,
        String title,
        String eventType,
        String description,
        String location,
        LocalDateTime startDate,
        int maxParticipants,
        int currentParticipants,
        int remainingPlaces,
        LocalDateTime createdAt
) {
}
