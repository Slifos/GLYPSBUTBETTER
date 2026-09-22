package com.campusconnect.event.repository;

import com.campusconnect.event.entity.Registration;
import com.campusconnect.event.entity.RegistrationStatus;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface RegistrationRepository extends JpaRepository<Registration, Long> {

    Optional<Registration> findByEventIdAndUserIdAndStatus(Long eventId, Long userId, RegistrationStatus status);

    long countByEventIdAndStatus(Long eventId, RegistrationStatus status);

    List<Registration> findByEventId(Long eventId);

    void deleteAllByEventId(Long eventId);
}
