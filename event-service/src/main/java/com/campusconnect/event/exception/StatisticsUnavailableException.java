package com.campusconnect.event.exception;

/** Wraps any gRPC failure when calling statistics-service, so the web layer never sees gRPC types. */
public class StatisticsUnavailableException extends RuntimeException {

    public StatisticsUnavailableException(String message, Throwable cause) {
        super(message, cause);
    }
}
