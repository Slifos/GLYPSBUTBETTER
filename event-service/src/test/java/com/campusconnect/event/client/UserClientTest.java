package com.campusconnect.event.client;

import com.campusconnect.event.exception.UserNotFoundException;
import com.campusconnect.event.exception.UserServiceUnavailableException;
import org.junit.jupiter.api.Test;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.test.web.client.MockRestServiceServer;
import org.springframework.web.client.RestClient;

import static org.assertj.core.api.Assertions.assertThatCode;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.springframework.test.web.client.match.MockRestRequestMatchers.requestTo;
import static org.springframework.test.web.client.response.MockRestResponseCreators.withStatus;
import static org.springframework.test.web.client.response.MockRestResponseCreators.withSuccess;

class UserClientTest {

    private static final String BASE_URL = "http://user-service";

    @Test
    void verifyExists_succeeds_whenUserFound() {
        RestClient.Builder builder = RestClient.builder();
        MockRestServiceServer server = MockRestServiceServer.bindTo(builder).build();
        server.expect(requestTo(BASE_URL + "/users/42")).andRespond(withSuccess("{}", MediaType.APPLICATION_JSON));

        UserClient client = new UserClient(builder, BASE_URL);

        assertThatCode(() -> client.verifyExists(42L)).doesNotThrowAnyException();
        server.verify();
    }

    @Test
    void verifyExists_throwsUserNotFound_when404() {
        RestClient.Builder builder = RestClient.builder();
        MockRestServiceServer server = MockRestServiceServer.bindTo(builder).build();
        server.expect(requestTo(BASE_URL + "/users/42")).andRespond(withStatus(HttpStatus.NOT_FOUND));

        UserClient client = new UserClient(builder, BASE_URL);

        assertThatThrownBy(() -> client.verifyExists(42L)).isInstanceOf(UserNotFoundException.class);
    }

    @Test
    void verifyExists_throwsUserServiceUnavailable_whenServerErrors() {
        RestClient.Builder builder = RestClient.builder();
        MockRestServiceServer server = MockRestServiceServer.bindTo(builder).build();
        server.expect(requestTo(BASE_URL + "/users/42")).andRespond(withStatus(HttpStatus.INTERNAL_SERVER_ERROR));

        UserClient client = new UserClient(builder, BASE_URL);

        assertThatThrownBy(() -> client.verifyExists(42L)).isInstanceOf(UserServiceUnavailableException.class);
    }
}
