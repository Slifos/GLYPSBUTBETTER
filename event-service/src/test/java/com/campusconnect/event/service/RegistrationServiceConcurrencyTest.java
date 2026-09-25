package com.campusconnect.event.service;

import com.campusconnect.event.client.UserClient;
import com.campusconnect.event.entity.Event;
import com.campusconnect.event.exception.EventFullException;
import com.campusconnect.event.repository.EventRepository;
import com.campusconnect.event.web.dto.RegistrationResponse;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.bean.override.mockito.MockitoBean;

import java.time.LocalDateTime;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Proves the pessimistic lock in {@link RegistrationService#register} actually
 * serializes concurrent registrations for the same event, using real
 * transactions against H2 rather than mocks.
 */
@SpringBootTest
class RegistrationServiceConcurrencyTest {

    @Autowired
    private RegistrationService registrationService;
    @Autowired
    private EventRepository eventRepository;
    @MockitoBean
    private UserClient userClient;

    @Test
    void register_allowsOnlyOneWinner_whenTwoUsersRaceForTheLastSpot() throws Exception {
        Event event = eventRepository.save(
                new Event("Smash tournament", "GAMING", "desc", "Room 1", LocalDateTime.now(), 1));
        Long eventId = event.getId();

        ExecutorService executor = Executors.newFixedThreadPool(2);
        CyclicBarrier barrier = new CyclicBarrier(2);
        try {
            Future<Object> first = executor.submit(attempt(barrier, eventId, 1L));
            Future<Object> second = executor.submit(attempt(barrier, eventId, 2L));
            List<Object> outcomes = List.of(first.get(), second.get());

            long successes = outcomes.stream().filter(o -> o instanceof RegistrationResponse).count();
            long fullExceptions = outcomes.stream().filter(o -> o instanceof EventFullException).count();

            assertThat(successes).isEqualTo(1);
            assertThat(fullExceptions).isEqualTo(1);
        } finally {
            executor.shutdownNow();
        }
    }

    private Callable<Object> attempt(CyclicBarrier barrier, Long eventId, Long userId) {
        return () -> {
            barrier.await();
            try {
                return registrationService.register(eventId, userId);
            } catch (EventFullException e) {
                return e;
            }
        };
    }
}
