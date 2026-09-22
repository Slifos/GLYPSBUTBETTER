package com.campusconnect.event.web.dto;

import com.campusconnect.event.entity.RegistrationStatus;

import java.time.LocalDateTime;

public record RegistrationResponse(
        Long id,
        Long eventId,
        Long userId,
        RegistrationStatus status,
        LocalDateTime createdAt,
        LocalDateTime updatedAt
) {
}
