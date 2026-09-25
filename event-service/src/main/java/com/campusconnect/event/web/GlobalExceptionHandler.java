package com.campusconnect.event.web;

import com.campusconnect.event.exception.DuplicateRegistrationException;
import com.campusconnect.event.exception.EventFullException;
import com.campusconnect.event.exception.EventNotFoundException;
import com.campusconnect.event.exception.InvalidEventCapacityException;
import com.campusconnect.event.exception.RegistrationNotFoundException;
import com.campusconnect.event.exception.StatisticsUnavailableException;
import com.campusconnect.event.exception.UserNotFoundException;
import com.campusconnect.event.exception.UserServiceUnavailableException;
import jakarta.servlet.http.HttpServletRequest;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.time.Instant;
import java.util.stream.Collectors;

/**
 * Translates business and validation exceptions into HTTP responses carrying
 * a message. Unexpected errors are logged with detail but never expose
 * internal information to the client.
 */
@Slf4j
@RestControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(EventNotFoundException.class)
    public ResponseEntity<ApiError> handleNotFound(EventNotFoundException ex, HttpServletRequest request) {
        return build(HttpStatus.NOT_FOUND, ex.getMessage(), request);
    }

    @ExceptionHandler(RegistrationNotFoundException.class)
    public ResponseEntity<ApiError> handleNotFound(RegistrationNotFoundException ex, HttpServletRequest request) {
        return build(HttpStatus.NOT_FOUND, ex.getMessage(), request);
    }

    @ExceptionHandler(UserNotFoundException.class)
    public ResponseEntity<ApiError> handleNotFound(UserNotFoundException ex, HttpServletRequest request) {
        return build(HttpStatus.NOT_FOUND, ex.getMessage(), request);
    }

    @ExceptionHandler(EventFullException.class)
    public ResponseEntity<ApiError> handleConflict(EventFullException ex, HttpServletRequest request) {
        return build(HttpStatus.CONFLICT, ex.getMessage(), request);
    }

    @ExceptionHandler(DuplicateRegistrationException.class)
    public ResponseEntity<ApiError> handleConflict(DuplicateRegistrationException ex, HttpServletRequest request) {
        return build(HttpStatus.CONFLICT, ex.getMessage(), request);
    }

    @ExceptionHandler(InvalidEventCapacityException.class)
    public ResponseEntity<ApiError> handleBadRequest(InvalidEventCapacityException ex, HttpServletRequest request) {
        return build(HttpStatus.BAD_REQUEST, ex.getMessage(), request);
    }

    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<ApiError> handleValidation(MethodArgumentNotValidException ex, HttpServletRequest request) {
        String message = ex.getBindingResult().getFieldErrors().stream()
                .map(fieldError -> fieldError.getField() + ": " + fieldError.getDefaultMessage())
                .collect(Collectors.joining("; "));
        return build(HttpStatus.BAD_REQUEST, message, request);
    }

    @ExceptionHandler(StatisticsUnavailableException.class)
    public ResponseEntity<ApiError> handleStatisticsUnavailable(StatisticsUnavailableException ex, HttpServletRequest request) {
        log.error("Statistics service call failed", ex);
        return build(HttpStatus.SERVICE_UNAVAILABLE, "Statistics service is currently unavailable", request);
    }

    @ExceptionHandler(UserServiceUnavailableException.class)
    public ResponseEntity<ApiError> handleUserServiceUnavailable(UserServiceUnavailableException ex, HttpServletRequest request) {
        log.error("User service call failed", ex);
        return build(HttpStatus.SERVICE_UNAVAILABLE, "User service is currently unavailable", request);
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity<ApiError> handleUnexpected(Exception ex, HttpServletRequest request) {
        log.error("Unexpected error handling {} {}", request.getMethod(), request.getRequestURI(), ex);
        return build(HttpStatus.INTERNAL_SERVER_ERROR, "An unexpected error occurred", request);
    }

    private ResponseEntity<ApiError> build(HttpStatus status, String message, HttpServletRequest request) {
        ApiError body = new ApiError(Instant.now(), status.value(), status.getReasonPhrase(), message, request.getRequestURI());
        return ResponseEntity.status(status).body(body);
    }
}
