package com.campusconnect.event.web;

import com.campusconnect.event.web.dto.EventCreateRequest;
import jakarta.validation.Validation;
import org.junit.jupiter.api.Test;

import java.time.LocalDateTime;

import static org.assertj.core.api.Assertions.assertThat;

class EventRequestValidationTest {
    @Test
    void rejectsPastEventDate() {
        try (var factory = Validation.buildDefaultValidatorFactory()) {
            var request = new EventCreateRequest(
                    "Past event", "TEST", null, null, LocalDateTime.now().minusDays(1), 2);
            assertThat(factory.getValidator().validate(request))
                    .anySatisfy(violation -> assertThat(violation.getPropertyPath().toString())
                            .isEqualTo("startDate"));
        }
    }
}
