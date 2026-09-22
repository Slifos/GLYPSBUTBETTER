package com.campusconnect.event.web.dto;

import jakarta.validation.constraints.NotNull;

public record RegistrationRequest(@NotNull Long userId) {
}
