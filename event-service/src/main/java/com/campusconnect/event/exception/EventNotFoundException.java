package com.campusconnect.event.exception;

public class EventNotFoundException extends RuntimeException {

    public EventNotFoundException(Long eventId) {
        super("Event not found: " + eventId);
    }
}
