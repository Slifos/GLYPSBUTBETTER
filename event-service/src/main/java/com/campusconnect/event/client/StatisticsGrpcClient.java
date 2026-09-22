package com.campusconnect.event.client;

import com.campusconnect.event.entity.Event;
import com.campusconnect.event.entity.Registration;
import com.campusconnect.event.exception.StatisticsUnavailableException;
import com.campusconnect.event.web.dto.CapacityAlertDto;
import com.campusconnect.event.web.dto.DashboardStatisticsDto;
import com.campusconnect.event.web.dto.EventAnalyticsDto;
import com.campusconnect.event.web.dto.EventComparisonDto;
import com.campusconnect.event.web.dto.EventTypeStatisticsDto;
import com.campusconnect.event.web.dto.GlobalStatisticsDto;
import com.campusconnect.event.web.dto.PopularEventDto;
import com.campusconnect.event.web.dto.RegistrationBucketDto;
import com.campusconnect.statistics.grpc.CapacityAlert;
import com.campusconnect.statistics.grpc.DashboardStatisticsRequest;
import com.campusconnect.statistics.grpc.EventAnalyticsRequest;
import com.campusconnect.statistics.grpc.EventData;
import com.campusconnect.statistics.grpc.GlobalStatistics;
import com.campusconnect.statistics.grpc.RegistrationBucket;
import com.campusconnect.statistics.grpc.RegistrationRecord;
import com.campusconnect.statistics.grpc.StatisticsServiceGrpc;
import io.grpc.StatusRuntimeException;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.time.Instant;
import java.time.ZoneOffset;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * Only place in the codebase that talks gRPC. Maps our JPA entities to the
 * {@code EventData} wire message and maps the responses back to plain DTOs,
 * so nothing above this layer depends on generated protobuf types.
 */
@Slf4j
@Component
@RequiredArgsConstructor
public class StatisticsGrpcClient {

    private static final long DEADLINE_SECONDS = 5;

    private final StatisticsServiceGrpc.StatisticsServiceBlockingStub statisticsStub;

    public EventAnalyticsDto analyzeEvent(Event event, List<Registration> registrations, Instant asOf) {
        EventAnalyticsRequest request = EventAnalyticsRequest.newBuilder()
                .setEvent(toEventData(event, registrations))
                .setAsOfEpochSeconds(asOf.getEpochSecond())
                .build();
        try {
            var response = statisticsStub.withDeadlineAfter(DEADLINE_SECONDS, TimeUnit.SECONDS)
                    .analyzeEvent(request);
            return toDto(response);
        } catch (StatusRuntimeException e) {
            log.error("AnalyzeEvent gRPC call failed for eventId={}: {}", event.getId(), e.getStatus(), e);
            throw new StatisticsUnavailableException("Statistics service unavailable", e);
        }
    }

    public DashboardStatisticsDto dashboard(List<Event> events,
                                             Map<Long, List<Registration>> registrationsByEvent,
                                             Instant asOf,
                                             int popularEventsLimit) {
        DashboardStatisticsRequest.Builder builder = DashboardStatisticsRequest.newBuilder()
                .setAsOfEpochSeconds(asOf.getEpochSecond())
                .setPopularEventsLimit(popularEventsLimit);
        for (Event event : events) {
            builder.addEvents(toEventData(event, registrationsByEvent.getOrDefault(event.getId(), List.of())));
        }
        try {
            var response = statisticsStub.withDeadlineAfter(DEADLINE_SECONDS, TimeUnit.SECONDS)
                    .getDashboardStatistics(builder.build());
            return toDto(response);
        } catch (StatusRuntimeException e) {
            log.error("GetDashboardStatistics gRPC call failed: {}", e.getStatus(), e);
            throw new StatisticsUnavailableException("Statistics service unavailable", e);
        }
    }

    private EventData toEventData(Event event, List<Registration> registrations) {
        EventData.Builder builder = EventData.newBuilder()
                .setEventId(event.getId())
                .setEventType(event.getEventType())
                .setMaxParticipants(event.getMaxParticipants());

        int current = 0;
        int cancelled = 0;
        int attended = 0;
        for (Registration registration : registrations) {
            switch (registration.getStatus()) {
                case ACTIVE -> current++;
                case ATTENDED -> {
                    current++;
                    attended++;
                }
                case CANCELLED -> cancelled++;
            }
            long timestamp = registration.getCreatedAt().toInstant(ZoneOffset.UTC).getEpochSecond();
            builder.addRegistrationHistory(RegistrationRecord.newBuilder()
                    .setTimestampEpochSeconds(timestamp)
                    .setRegistrations(1)
                    .build());
        }
        return builder
                .setCurrentParticipants(current)
                .setCancelledParticipants(cancelled)
                .setAttendedParticipants(attended)
                .build();
    }

    private EventAnalyticsDto toDto(com.campusconnect.statistics.grpc.EventAnalyticsResponse response) {
        return new EventAnalyticsDto(
                response.getEventId(),
                response.getRemainingPlaces(),
                response.getOccupancyRate(),
                response.getFull(),
                response.getCancellationRate(),
                response.getAttendedParticipants(),
                response.getAttendanceRate(),
                response.getDailyRegistrationsList().stream().map(this::toDto).toList(),
                response.getWeeklyRegistrationsList().stream().map(this::toDto).toList(),
                response.getRegistrationsPerDay(),
                response.getEstimatedFullAtEpochSeconds() == 0
                        ? null : Instant.ofEpochSecond(response.getEstimatedFullAtEpochSeconds()),
                response.getAlertsList().stream().map(this::toDto).toList()
        );
    }

    private RegistrationBucketDto toDto(RegistrationBucket bucket) {
        return new RegistrationBucketDto(Instant.ofEpochSecond(bucket.getPeriodStartEpochSeconds()), bucket.getRegistrations());
    }

    private CapacityAlertDto toDto(CapacityAlert alert) {
        return new CapacityAlertDto(alert.getLevel().name(), alert.getThreshold(), alert.getMessage());
    }

    private DashboardStatisticsDto toDto(com.campusconnect.statistics.grpc.DashboardStatisticsResponse response) {
        List<EventComparisonDto> comparisons = response.getComparisonsList().stream()
                .map(c -> new EventComparisonDto(
                        c.getEventId(), c.getEventType(), c.getCurrentParticipants(), c.getOccupancyRate(),
                        c.getCancellationRate(), c.getAttendanceRate(), c.getRegistrationsPerDay()))
                .toList();
        List<PopularEventDto> popular = response.getPopularEventsList().stream()
                .map(p -> new PopularEventDto(
                        p.getEventId(), p.getEventType(), p.getCurrentParticipants(), p.getOccupancyRate(), p.getRank()))
                .toList();
        List<EventTypeStatisticsDto> byType = response.getStatisticsByTypeList().stream()
                .map(s -> new EventTypeStatisticsDto(
                        s.getEventType(), s.getEventCount(), s.getAverageParticipants(),
                        s.getAverageAttendanceRate(), s.getRecommendedCapacity()))
                .toList();
        GlobalStatistics g = response.getGlobal();
        GlobalStatisticsDto global = new GlobalStatisticsDto(
                g.getTotalEvents(), g.getFullEvents(), g.getTotalCapacity(), g.getTotalCurrentParticipants(),
                g.getTotalCancelledParticipants(), g.getTotalAttendedParticipants(), g.getAverageParticipantsPerEvent(),
                g.getOverallOccupancyRate(), g.getOverallCancellationRate(), g.getOverallAttendanceRate()
        );
        return new DashboardStatisticsDto(comparisons, popular, byType, global);
    }
}
