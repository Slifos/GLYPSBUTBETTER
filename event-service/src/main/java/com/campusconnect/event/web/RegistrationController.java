package com.campusconnect.event.web;

import com.campusconnect.event.service.RegistrationService;
import com.campusconnect.event.web.dto.RegistrationRequest;
import com.campusconnect.event.web.dto.RegistrationResponse;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController
@RequestMapping("/events/{eventId}/registrations")
@RequiredArgsConstructor
public class RegistrationController {

    private final RegistrationService registrationService;

    @PostMapping
    public ResponseEntity<RegistrationResponse> register(@PathVariable Long eventId,
                                                           @Valid @RequestBody RegistrationRequest request) {
        return ResponseEntity.status(HttpStatus.CREATED).body(registrationService.register(eventId, request.userId()));
    }

    @GetMapping
    public List<RegistrationResponse> list(@PathVariable Long eventId) {
        return registrationService.listForEvent(eventId);
    }

    @DeleteMapping("/{userId}")
    public ResponseEntity<Void> cancel(@PathVariable Long eventId, @PathVariable Long userId) {
        registrationService.cancel(eventId, userId);
        return ResponseEntity.noContent().build();
    }

    @PostMapping("/{userId}/attendance")
    public RegistrationResponse markAttended(@PathVariable Long eventId, @PathVariable Long userId) {
        return registrationService.markAttended(eventId, userId);
    }
}
