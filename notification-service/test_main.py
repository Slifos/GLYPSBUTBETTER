import os
import tempfile

os.environ["DATABASE_URL"] = f"sqlite:///{tempfile.mkdtemp()}/notifications.db"
os.environ["NOTIFICATION_CONSUMER_ENABLED"] = "false"

from fastapi.testclient import TestClient

import main


def test_notification_is_stored_once_and_visible_to_recipient():
    with TestClient(main.app) as client:
        payload = {
            "messageId": "ab844bda-d431-4052-821f-1202df5c44b3",
            "userId": 42,
            "eventId": 5,
            "kind": "REGISTRATION_CONFIRMED",
            "message": "You are registered for Smash",
        }
        main.store_notification(payload)
        main.store_notification(payload)

        response = client.get("/notifications/users/42")
        assert response.status_code == 200
        assert len(response.json()) == 1
        assert response.json()[0]["message"] == payload["message"]
        assert client.get("/notifications/users/99").json() == []
