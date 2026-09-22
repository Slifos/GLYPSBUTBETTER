package com.campusconnect.event.web.dto;

import java.time.Instant;

public record RegistrationBucketDto(Instant periodStart, int registrations) {
}
