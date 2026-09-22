package com.campusconnect.event.exception;

public class DuplicateRegistrationException extends RuntimeException {

    public DuplicateRegistrationException(Long eventId, Long userId) {
        super("User " + userId + " is already registered for event " + eventId);
    }
}
