package com.campusconnect.event.notification;

import org.junit.jupiter.api.Test;
import org.mockito.ArgumentCaptor;
import org.springframework.amqp.core.MessagePostProcessor;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import tools.jackson.databind.ObjectMapper;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

class NotificationQueuePublisherTest {
    @Test
    void publishesJsonMatchingNotificationConsumerContract() {
        RabbitTemplate rabbitTemplate = mock(RabbitTemplate.class);
        NotificationQueuePublisher publisher = new NotificationQueuePublisher(rabbitTemplate, new ObjectMapper());
        NotificationMessage message = NotificationMessage.of(42L, 7L, "EVENT_FULL", "Smash is now full");

        publisher.publish(message);

        ArgumentCaptor<String> body = ArgumentCaptor.forClass(String.class);
        verify(rabbitTemplate).convertAndSend(
                eq(NotificationQueuePublisher.EXCHANGE), eq(NotificationQueuePublisher.ROUTING_KEY),
                body.capture(), any(MessagePostProcessor.class));
        assertThat(body.getValue()).contains("\"userId\":42", "\"eventId\":7", "\"kind\":\"EVENT_FULL\"");
    }
}
