package com.campusconnect.event.service;

import com.campusconnect.event.client.UserClient;
import com.campusconnect.event.entity.Event;
import com.campusconnect.event.entity.Registration;
import com.campusconnect.event.entity.RegistrationStatus;
import com.campusconnect.event.exception.DuplicateRegistrationException;
import com.campusconnect.event.exception.EventFullException;
import com.campusconnect.event.exception.RegistrationNotFoundException;
import com.campusconnect.event.repository.RegistrationRepository;
import com.campusconnect.event.web.dto.RegistrationResponse;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

/**
 * Business logic for registrations: enforces one active registration per user
 * per event and the event's capacity limit.
 */
@Slf4j
@Service
@RequiredArgsConstructor
@Transactional(readOnly = true)
public class RegistrationService {

    private final RegistrationRepository registrationRepository;
    private final EventService eventService;
    private final UserClient userClient;

    @Transactional
    public RegistrationResponse register(Long eventId, Long userId) {
        Event event = eventService.findEventOrThrow(eventId);
        userClient.verifyExists(userId);

        registrationRepository.findByEventIdAndUserIdAndStatus(eventId, userId, RegistrationStatus.ACTIVE)
                .ifPresent(existing -> {
                    throw new DuplicateRegistrationException(eventId, userId);
                });

        long current = eventService.countCurrentParticipants(eventId);
        if (current >= event.getMaxParticipants()) {
            throw new EventFullException(eventId);
        }

        Registration registration = registrationRepository.save(new Registration(event, userId));
        log.info("User {} registered for event {}", userId, eventId);
        return toResponse(registration);
    }

    @Transactional
    public void cancel(Long eventId, Long userId) {
        Registration registration = findActiveOrThrow(eventId, userId);
        registration.cancel();
        log.info("User {} cancelled registration for event {}", userId, eventId);
    }

    @Transactional
    public RegistrationResponse markAttended(Long eventId, Long userId) {
        Registration registration = findActiveOrThrow(eventId, userId);
        registration.markAttended();
        log.info("User {} marked attended for event {}", userId, eventId);
        return toResponse(registration);
    }

    public List<RegistrationResponse> listForEvent(Long eventId) {
        eventService.findEventOrThrow(eventId);
        return registrationRepository.findByEventId(eventId).stream().map(this::toResponse).toList();
    }

    private Registration findActiveOrThrow(Long eventId, Long userId) {
        return registrationRepository.findByEventIdAndUserIdAndStatus(eventId, userId, RegistrationStatus.ACTIVE)
                .orElseThrow(() -> new RegistrationNotFoundException(eventId, userId));
    }

    private RegistrationResponse toResponse(Registration registration) {
        return new RegistrationResponse(
                registration.getId(),
                registration.getEvent().getId(),
                registration.getUserId(),
                registration.getStatus(),
                registration.getCreatedAt(),
                registration.getUpdatedAt()
        );
    }
}
