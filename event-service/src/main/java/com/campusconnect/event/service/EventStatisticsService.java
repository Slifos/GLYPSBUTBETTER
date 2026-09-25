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

import java.time.Duration;
import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;
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

    // Dashboard re-ships every event's full registration history on every call;
    // a short TTL trades a few seconds of staleness for not recomputing that on
    // every request. Move to incrementally-maintained aggregates if this needs
    // to be fresher or traffic grows enough for 10s of staleness to matter.
    private static final Duration DASHBOARD_CACHE_TTL = Duration.ofSeconds(10);

    private final EventService eventService;
    private final EventRepository eventRepository;
    private final RegistrationRepository registrationRepository;
    private final StatisticsGrpcClient statisticsGrpcClient;

    private final AtomicReference<CachedDashboard> dashboardCache = new AtomicReference<>();

    private record CachedDashboard(int popularEventsLimit, Instant expiresAt, DashboardStatisticsDto dto) {
    }

    public EventAnalyticsDto getEventStatistics(Long eventId) {
        Event event = eventService.findEventOrThrow(eventId);
        List<Registration> registrations = registrationRepository.findByEventId(eventId);
        return statisticsGrpcClient.analyzeEvent(event, registrations, Instant.now());
    }

    public DashboardStatisticsDto getDashboardStatistics(int popularEventsLimit) {
        Instant now = Instant.now();
        CachedDashboard cached = dashboardCache.get();
        if (cached != null && cached.popularEventsLimit() == popularEventsLimit && now.isBefore(cached.expiresAt())) {
            return cached.dto();
        }

        List<Event> events = eventRepository.findAll();
        List<Long> eventIds = events.stream().map(Event::getId).toList();
        Map<Long, List<Registration>> registrationsByEvent = registrationRepository.findByEventIdIn(eventIds).stream()
                .collect(Collectors.groupingBy(r -> r.getEvent().getId()));
        DashboardStatisticsDto dto = statisticsGrpcClient.dashboard(events, registrationsByEvent, now, popularEventsLimit);
        dashboardCache.set(new CachedDashboard(popularEventsLimit, now.plus(DASHBOARD_CACHE_TTL), dto));
        return dto;
    }
}
