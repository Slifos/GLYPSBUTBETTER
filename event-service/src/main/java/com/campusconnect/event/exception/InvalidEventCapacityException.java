package com.campusconnect.event.exception;

public class InvalidEventCapacityException extends RuntimeException {

    public InvalidEventCapacityException(int requestedCapacity, long currentParticipants) {
        super("Cannot set max participants to " + requestedCapacity
                + ": there are already " + currentParticipants + " current participants");
    }
}
