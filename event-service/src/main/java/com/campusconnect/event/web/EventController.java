package com.campusconnect.event.web;

import com.campusconnect.event.service.EventService;
import com.campusconnect.event.service.EventStatisticsService;
import com.campusconnect.event.web.dto.DashboardStatisticsDto;
import com.campusconnect.event.web.dto.EventAnalyticsDto;
import com.campusconnect.event.web.dto.EventCreateRequest;
import com.campusconnect.event.web.dto.EventResponse;
import com.campusconnect.event.web.dto.EventUpdateRequest;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController
@RequiredArgsConstructor
public class EventController {

    private final EventService eventService;
    private final EventStatisticsService eventStatisticsService;

    @PostMapping("/events")
    public ResponseEntity<EventResponse> createEvent(@Valid @RequestBody EventCreateRequest request) {
        return ResponseEntity.status(HttpStatus.CREATED).body(eventService.createEvent(request));
    }

    @GetMapping("/events")
    public List<EventResponse> listEvents() {
        return eventService.listEvents();
    }

    @GetMapping("/events/{eventId}")
    public EventResponse getEvent(@PathVariable Long eventId) {
        return eventService.getEvent(eventId);
    }

    @PutMapping("/events/{eventId}")
    public EventResponse updateEvent(@PathVariable Long eventId, @Valid @RequestBody EventUpdateRequest request) {
        return eventService.updateEvent(eventId, request);
    }

    @DeleteMapping("/events/{eventId}")
    public ResponseEntity<Void> deleteEvent(@PathVariable Long eventId) {
        eventService.deleteEvent(eventId);
        return ResponseEntity.noContent().build();
    }

    /** Detailed analytics for a single event, computed by statistics-service over gRPC. */
    @GetMapping("/events/{eventId}/statistics")
    public EventAnalyticsDto getEventStatistics(@PathVariable Long eventId) {
        return eventStatisticsService.getEventStatistics(eventId);
    }

    /** Cross-event dashboard, computed by statistics-service over gRPC. */
    @GetMapping("/events/statistics/dashboard")
    public DashboardStatisticsDto getDashboardStatistics(@RequestParam(defaultValue = "0") int popularEventsLimit) {
        return eventStatisticsService.getDashboardStatistics(popularEventsLimit);
    }
}
