package com.campusconnect.event.entity;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import jakarta.persistence.Table;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.LocalDateTime;

/**
 * An event that people can register for. Registrations, cancellations and
 * attendance are tracked separately by {@link Registration}; this entity only
 * holds the event's own data.
 */
@Entity
@Table(name = "events")
@Getter
@Setter
@NoArgsConstructor
public class Event {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String title;

    @Column(name = "event_type", nullable = false)
    private String eventType;

    @Column(length = 2000)
    private String description;

    private String location;

    @Column(name = "start_date", nullable = false)
    private LocalDateTime startDate;

    @Column(name = "max_participants", nullable = false)
    private Integer maxParticipants;

    @Column(name = "created_at", nullable = false, updatable = false)
    private LocalDateTime createdAt;

    public Event(String title, String eventType, String description, String location,
                 LocalDateTime startDate, Integer maxParticipants) {
        this.title = title;
        this.eventType = eventType;
        this.description = description;
        this.location = location;
        this.startDate = startDate;
        this.maxParticipants = maxParticipants;
    }

    @PrePersist
    void onCreate() {
        this.createdAt = LocalDateTime.now();
    }
}
