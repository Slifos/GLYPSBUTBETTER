package com.campusconnect.event.service;

import com.campusconnect.event.client.StatisticsGrpcClient;
import com.campusconnect.event.entity.Event;
import com.campusconnect.event.entity.Registration;
import com.campusconnect.event.repository.EventRepository;
import com.campusconnect.event.repository.RegistrationRepository;
import com.campusconnect.event.web.dto.DashboardStatisticsDto;
import com.campusconnect.event.web.dto.EventAnalyticsDto;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Fetches events and their registrations, then delegates the actual
 * statistics computation to statistics-service over gRPC via
 * {@link StatisticsGrpcClient}. Kept separate from {@link EventService} so
 * that service stays free of gRPC concerns.
 */
@Service
@RequiredArgsConstructor
@Transactional(readOnly = true)
public class EventStatisticsService {

    private final EventService eventService;
    private final EventRepository eventRepository;
    private final RegistrationRepository registrationRepository;
    private final StatisticsGrpcClient statisticsGrpcClient;

    public EventAnalyticsDto getEventStatistics(Long eventId) {
        Event event = eventService.findEventOrThrow(eventId);
        List<Registration> registrations = registrationRepository.findByEventId(eventId);
        return statisticsGrpcClient.analyzeEvent(event, registrations, Instant.now());
    }

    public DashboardStatisticsDto getDashboardStatistics(int popularEventsLimit) {
        List<Event> events = eventRepository.findAll();
        Map<Long, List<Registration>> registrationsByEvent = events.stream()
                .collect(Collectors.toMap(Event::getId, e -> registrationRepository.findByEventId(e.getId())));
        return statisticsGrpcClient.dashboard(events, registrationsByEvent, Instant.now(), popularEventsLimit);
    }
}
