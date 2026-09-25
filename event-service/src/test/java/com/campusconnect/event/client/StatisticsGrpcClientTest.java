package com.campusconnect.event.client;

import com.campusconnect.event.entity.Event;
import com.campusconnect.event.exception.StatisticsUnavailableException;
import com.campusconnect.event.web.dto.EventAnalyticsDto;
import com.campusconnect.statistics.grpc.EventAnalyticsRequest;
import com.campusconnect.statistics.grpc.EventAnalyticsResponse;
import com.campusconnect.statistics.grpc.StatisticsServiceGrpc;
import io.grpc.ManagedChannel;
import io.grpc.Server;
import io.grpc.Status;
import io.grpc.inprocess.InProcessChannelBuilder;
import io.grpc.inprocess.InProcessServerBuilder;
import io.grpc.stub.StreamObserver;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;

import java.time.Instant;
import java.time.LocalDateTime;
import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

/** Exercises the client against a real (in-process) gRPC server, no mocks. */
class StatisticsGrpcClientTest {

    private Server server;
    private ManagedChannel channel;

    @AfterEach
    void tearDown() {
        if (channel != null) {
            channel.shutdownNow();
        }
        if (server != null) {
            server.shutdownNow();
        }
    }

    @Test
    void analyzeEvent_mapsResponseToDto() throws Exception {
        StatisticsGrpcClient client = startClient(new StatisticsServiceGrpc.StatisticsServiceImplBase() {
            @Override
            public void analyzeEvent(EventAnalyticsRequest request, StreamObserver<EventAnalyticsResponse> responseObserver) {
                responseObserver.onNext(EventAnalyticsResponse.newBuilder()
                        .setEventId(request.getEvent().getEventId())
                        .setRemainingPlaces(3)
                        .setOccupancyRate(70.0)
                        .setFull(false)
                        .build());
                responseObserver.onCompleted();
            }
        });

        EventAnalyticsDto dto = client.analyzeEvent(sampleEvent(), List.of(), Instant.now());

        assertThat(dto.eventId()).isEqualTo(1L);
        assertThat(dto.remainingPlaces()).isEqualTo(3);
        assertThat(dto.occupancyRate()).isEqualTo(70.0);
        assertThat(dto.full()).isFalse();
    }

    @Test
    void analyzeEvent_throwsStatisticsUnavailable_onGrpcError() throws Exception {
        StatisticsGrpcClient client = startClient(new StatisticsServiceGrpc.StatisticsServiceImplBase() {
            @Override
            public void analyzeEvent(EventAnalyticsRequest request, StreamObserver<EventAnalyticsResponse> responseObserver) {
                responseObserver.onError(Status.INTERNAL.withDescription("boom").asRuntimeException());
            }
        });

        assertThatThrownBy(() -> client.analyzeEvent(sampleEvent(), List.of(), Instant.now()))
                .isInstanceOf(StatisticsUnavailableException.class);
    }

    private Event sampleEvent() {
        Event event = new Event("Smash tournament", "GAMING", "desc", "Room 1", LocalDateTime.now(), 10);
        event.setId(1L);
        return event;
    }

    private StatisticsGrpcClient startClient(StatisticsServiceGrpc.StatisticsServiceImplBase impl) throws Exception {
        String name = InProcessServerBuilder.generateName();
        server = InProcessServerBuilder.forName(name).directExecutor().addService(impl).build().start();
        channel = InProcessChannelBuilder.forName(name).directExecutor().build();
        return new StatisticsGrpcClient(StatisticsServiceGrpc.newBlockingStub(channel));
    }
}
