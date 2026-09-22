package com.campusconnect.event.entity;

/**
 * Lifecycle of a single {@link Registration}.
 *
 * <ul>
 *   <li>{@code ACTIVE} - the user is registered and counted as a current participant.</li>
 *   <li>{@code ATTENDED} - the user showed up; still a current participant, additionally
 *       counted in attendance statistics.</li>
 *   <li>{@code CANCELLED} - the user withdrew; no longer a current participant.</li>
 * </ul>
 */
public enum RegistrationStatus {
    ACTIVE,
    ATTENDED,
    CANCELLED
}
