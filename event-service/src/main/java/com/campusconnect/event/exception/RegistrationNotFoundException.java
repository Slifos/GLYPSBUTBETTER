package com.campusconnect.event.exception;

public class RegistrationNotFoundException extends RuntimeException {

    public RegistrationNotFoundException(Long eventId, Long userId) {
        super("No active registration for user " + userId + " on event " + eventId);
    }
}
