package com.campusconnect.event.exception;

/** Wraps any HTTP failure when calling user-service, so the web layer never sees client types. */
public class UserServiceUnavailableException extends RuntimeException {

    public UserServiceUnavailableException(String message, Throwable cause) {
        super(message, cause);
    }
}
