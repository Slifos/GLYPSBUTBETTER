package com.campusconnect.event.web.dto;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Future;
import jakarta.validation.constraints.Positive;
import jakarta.validation.constraints.Size;

import java.time.LocalDateTime;

public record EventCreateRequest(
        @NotBlank String title,
        @NotBlank String eventType,
        @Size(max = 2000) String description,
        String location,
        @NotNull @Future LocalDateTime startDate,
        @NotNull @Positive Integer maxParticipants
) {
}
