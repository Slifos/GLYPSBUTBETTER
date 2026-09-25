package com.campusconnect.event.client;

import com.campusconnect.event.exception.UserNotFoundException;
import com.campusconnect.event.exception.UserServiceUnavailableException;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.RestClient;
import org.springframework.web.client.RestClientException;

/** Only place in the codebase that talks to user-service. */
@Slf4j
@Component
public class UserClient {

    private final RestClient restClient;

    public UserClient(RestClient.Builder builder, @Value("${user-service.base-url}") String baseUrl) {
        this.restClient = builder.baseUrl(baseUrl).build();
    }

    public void verifyExists(Long userId) {
        try {
            restClient.get().uri("/users/{id}", userId).retrieve().toBodilessEntity();
        } catch (HttpClientErrorException.NotFound e) {
            throw new UserNotFoundException(userId);
        } catch (RestClientException e) {
            log.error("User-service call failed for userId={}", userId, e);
            throw new UserServiceUnavailableException("User service unavailable", e);
        }
    }
}
