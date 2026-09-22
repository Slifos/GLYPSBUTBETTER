package com.campusconnect.event.exception;

public class EventFullException extends RuntimeException {

    public EventFullException(Long eventId) {
        super("Event " + eventId + " has reached its maximum capacity");
    }
}
