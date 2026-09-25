package com.campusconnect.event.service;

import com.campusconnect.event.entity.Event;
import com.campusconnect.event.entity.RegistrationStatus;
import com.campusconnect.event.exception.EventNotFoundException;
import com.campusconnect.event.exception.InvalidEventCapacityException;
import com.campusconnect.event.repository.EventRepository;
import com.campusconnect.event.repository.RegistrationRepository;
import com.campusconnect.event.web.dto.EventCreateRequest;
import com.campusconnect.event.web.dto.EventResponse;
import com.campusconnect.event.web.dto.EventUpdateRequest;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

/**
 * Business logic for events. Holds no gRPC or HTTP concerns: controllers call
 * this service, and this service calls the repositories.
 */
@Slf4j
@Service
@RequiredArgsConstructor
@Transactional(readOnly = true)
public class EventService {

    private final EventRepository eventRepository;
    private final RegistrationRepository registrationRepository;

    @Transactional
    public EventResponse createEvent(EventCreateRequest request) {
        Event event = new Event(
                request.title(),
                request.eventType(),
                request.description(),
                request.location(),
                request.startDate(),
                request.maxParticipants()
        );
        event = eventRepository.save(event);
        log.info("Created event id={} title={}", event.getId(), event.getTitle());
        return toResponse(event);
    }

    public EventResponse getEvent(Long eventId) {
        return toResponse(findEventOrThrow(eventId));
    }

    public List<EventResponse> listEvents() {
        return eventRepository.findAll().stream().map(this::toResponse).toList();
    }

    @Transactional
    public EventResponse updateEvent(Long eventId, EventUpdateRequest request) {
        Event event = findEventForUpdateOrThrow(eventId);
        long currentParticipants = countCurrentParticipants(eventId);
        if (request.maxParticipants() < currentParticipants) {
            throw new InvalidEventCapacityException(request.maxParticipants(), currentParticipants);
        }
        event.setTitle(request.title());
        event.setEventType(request.eventType());
        event.setDescription(request.description());
        event.setLocation(request.location());
        event.setStartDate(request.startDate());
        event.setMaxParticipants(request.maxParticipants());
        log.info("Updated event id={}", eventId);
        return toResponse(event);
    }

    @Transactional
    public void deleteEvent(Long eventId) {
        findEventOrThrow(eventId);
        registrationRepository.deleteAllByEventId(eventId);
        eventRepository.deleteById(eventId);
        log.info("Deleted event id={}", eventId);
    }

    Event findEventOrThrow(Long eventId) {
        return eventRepository.findById(eventId)
                .orElseThrow(() -> new EventNotFoundException(eventId));
    }

    /** Locks the event row for the rest of the transaction; use where the caller checks-then-writes capacity/registrations. */
    @Transactional
    Event findEventForUpdateOrThrow(Long eventId) {
        return eventRepository.findByIdForUpdate(eventId)
                .orElseThrow(() -> new EventNotFoundException(eventId));
    }

    long countCurrentParticipants(Long eventId) {
        return registrationRepository.countByEventIdAndStatus(eventId, RegistrationStatus.ACTIVE)
                + registrationRepository.countByEventIdAndStatus(eventId, RegistrationStatus.ATTENDED);
    }

    private EventResponse toResponse(Event event) {
        long current = countCurrentParticipants(event.getId());
        int remaining = (int) Math.max(0, event.getMaxParticipants() - current);
        return new EventResponse(
                event.getId(),
                event.getTitle(),
                event.getEventType(),
                event.getDescription(),
                event.getLocation(),
                event.getStartDate(),
                event.getMaxParticipants(),
                (int) current,
                remaining,
                event.getCreatedAt()
        );
    }
}
