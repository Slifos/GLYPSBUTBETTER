package com.campusconnect.event.notification;

import java.util.UUID;

public record NotificationMessage(String messageId, Long userId, Long eventId, String kind, String message) {
    public static NotificationMessage of(Long userId, Long eventId, String kind, String message) {
        return new NotificationMessage(UUID.randomUUID().toString(), userId, eventId, kind, message);
    }
}
