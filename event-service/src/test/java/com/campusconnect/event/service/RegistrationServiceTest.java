package com.campusconnect.event.service;

import com.campusconnect.event.client.UserClient;
import com.campusconnect.event.entity.Event;
import com.campusconnect.event.entity.Registration;
import com.campusconnect.event.entity.RegistrationStatus;
import com.campusconnect.event.exception.DuplicateRegistrationException;
import com.campusconnect.event.exception.EventFullException;
import com.campusconnect.event.exception.RegistrationNotFoundException;
import com.campusconnect.event.exception.UserNotFoundException;
import com.campusconnect.event.repository.RegistrationRepository;
import com.campusconnect.event.web.dto.RegistrationResponse;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

@ExtendWith(MockitoExtension.class)
class RegistrationServiceTest {

    private static final Long EVENT_ID = 1L;
    private static final Long USER_ID = 42L;

    @Mock
    private RegistrationRepository registrationRepository;
    @Mock
    private EventService eventService;
    @Mock
    private UserClient userClient;

    @InjectMocks
    private RegistrationService registrationService;

    private Event event;

    @BeforeEach
    void setUp() {
        event = new Event("Smash tournament", "GAMING", "desc", "Room 1", LocalDateTime.now(), 2);
        event.setId(EVENT_ID);
    }

    @Test
    void register_savesActiveRegistration_whenCapacityAvailable() {
        when(eventService.findEventOrThrow(EVENT_ID)).thenReturn(event);
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.empty());
        when(eventService.countCurrentParticipants(EVENT_ID)).thenReturn(1L);
        when(registrationRepository.save(any(Registration.class))).thenAnswer(invocation -> {
            Registration registration = invocation.getArgument(0);
            registration.setId(99L);
            return registration;
        });

        RegistrationResponse response = registrationService.register(EVENT_ID, USER_ID);

        assertThat(response.status()).isEqualTo(RegistrationStatus.ACTIVE);
        assertThat(response.userId()).isEqualTo(USER_ID);
        verify(userClient).verifyExists(USER_ID);
    }

    @Test
    void register_throwsEventFull_whenCapacityReached() {
        when(eventService.findEventOrThrow(EVENT_ID)).thenReturn(event);
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.empty());
        when(eventService.countCurrentParticipants(EVENT_ID)).thenReturn(2L);

        assertThatThrownBy(() -> registrationService.register(EVENT_ID, USER_ID))
                .isInstanceOf(EventFullException.class);
        verify(registrationRepository, never()).save(any());
    }

    @Test
    void register_throwsDuplicate_whenAlreadyActivelyRegistered() {
        when(eventService.findEventOrThrow(EVENT_ID)).thenReturn(event);
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.of(new Registration(event, USER_ID)));

        assertThatThrownBy(() -> registrationService.register(EVENT_ID, USER_ID))
                .isInstanceOf(DuplicateRegistrationException.class);
        verify(registrationRepository, never()).save(any());
    }

    @Test
    void register_propagatesUserNotFound_withoutTouchingRegistrations() {
        when(eventService.findEventOrThrow(EVENT_ID)).thenReturn(event);
        org.mockito.Mockito.doThrow(new UserNotFoundException(USER_ID)).when(userClient).verifyExists(USER_ID);

        assertThatThrownBy(() -> registrationService.register(EVENT_ID, USER_ID))
                .isInstanceOf(UserNotFoundException.class);
        verify(registrationRepository, never()).findByEventIdAndUserIdAndStatus(any(), any(), any());
        verify(registrationRepository, never()).save(any());
    }

    @Test
    void cancel_marksRegistrationCancelled_whenActive() {
        Registration registration = new Registration(event, USER_ID);
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.of(registration));

        registrationService.cancel(EVENT_ID, USER_ID);

        assertThat(registration.getStatus()).isEqualTo(RegistrationStatus.CANCELLED);
    }

    @Test
    void cancel_throwsRegistrationNotFound_whenNoActiveRegistration() {
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.empty());

        assertThatThrownBy(() -> registrationService.cancel(EVENT_ID, USER_ID))
                .isInstanceOf(RegistrationNotFoundException.class);
    }

    @Test
    void markAttended_marksRegistrationAttended_whenActive() {
        Registration registration = new Registration(event, USER_ID);
        when(registrationRepository.findByEventIdAndUserIdAndStatus(EVENT_ID, USER_ID, RegistrationStatus.ACTIVE))
                .thenReturn(Optional.of(registration));

        RegistrationResponse response = registrationService.markAttended(EVENT_ID, USER_ID);

        assertThat(response.status()).isEqualTo(RegistrationStatus.ATTENDED);
    }

    @Test
    void listForEvent_returnsMappedRegistrations() {
        Registration registration = new Registration(event, USER_ID);
        when(eventService.findEventOrThrow(EVENT_ID)).thenReturn(event);
        when(registrationRepository.findByEventId(EVENT_ID)).thenReturn(List.of(registration));

        List<RegistrationResponse> responses = registrationService.listForEvent(EVENT_ID);

        assertThat(responses).hasSize(1);
        assertThat(responses.get(0).userId()).isEqualTo(USER_ID);
    }
}
