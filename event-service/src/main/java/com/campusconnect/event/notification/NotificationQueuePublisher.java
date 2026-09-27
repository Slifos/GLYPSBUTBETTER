package com.campusconnect.event.notification;

import org.springframework.amqp.core.Binding;
import org.springframework.amqp.core.BindingBuilder;
import org.springframework.amqp.core.DirectExchange;
import org.springframework.amqp.core.Queue;
import org.springframework.amqp.rabbit.core.RabbitTemplate;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.stereotype.Component;
import org.springframework.transaction.event.TransactionPhase;
import org.springframework.transaction.event.TransactionalEventListener;
import tools.jackson.databind.ObjectMapper;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

@Slf4j
@Component
@RequiredArgsConstructor
@ConditionalOnProperty(name = "notification.enabled", havingValue = "true", matchIfMissing = true)
public class NotificationQueuePublisher {
    public static final String EXCHANGE = "campusconnect.notifications";
    public static final String QUEUE = "notification-service";
    public static final String ROUTING_KEY = "notification";

    private final RabbitTemplate rabbitTemplate;
    private final ObjectMapper objectMapper;

    @TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
    public void publish(NotificationMessage notification) {
        try {
            rabbitTemplate.convertAndSend(EXCHANGE, ROUTING_KEY, objectMapper.writeValueAsString(notification),
                    message -> {
                        message.getMessageProperties().setDeliveryMode(org.springframework.amqp.core.MessageDeliveryMode.PERSISTENT);
                        message.getMessageProperties().setContentType("application/json");
                        return message;
                    });
        } catch (RuntimeException e) {
            log.error("Could not publish notification messageId={}", notification.messageId(), e);
        }
    }

    @Configuration
    @ConditionalOnProperty(name = "notification.enabled", havingValue = "true", matchIfMissing = true)
    static class RabbitTopology {
        @Bean
        DirectExchange notificationsExchange() {
            return new DirectExchange(EXCHANGE, true, false);
        }

        @Bean
        Queue notificationsQueue() {
            return new Queue(QUEUE, true);
        }

        @Bean
        Binding notificationsBinding(Queue notificationsQueue, DirectExchange notificationsExchange) {
            return BindingBuilder.bind(notificationsQueue).to(notificationsExchange).with(ROUTING_KEY);
        }
    }
}
