import os
import tempfile

os.environ["DATABASE_URL"] = f"sqlite:///{tempfile.mkdtemp()}/t.db"

from fastapi.testclient import TestClient

from app.main import app


def test_user_crud_flow_and_http_errors():
    with TestClient(app) as client:
        created = client.post("/users", json={"name": "Ana", "email": "ana@efrei.net"})
        assert created.status_code == 201
        user_id = created.json()["id"]

        assert client.get(f"/users/{user_id}").json()["name"] == "Ana"
        assert len(client.get("/users").json()) == 1

        duplicate = client.post("/users", json={"name": "Other", "email": "ana@efrei.net"})
        assert duplicate.status_code == 400
        assert duplicate.json()["detail"] == "EMAIL_ALREADY_USED"

        missing = client.get("/users/999")
        assert missing.status_code == 404
        assert missing.json()["detail"] == "USER_NOT_FOUND"

        updated = client.put(f"/users/{user_id}", json={"name": "Ana Updated", "email": "ana2@efrei.net"})
        assert updated.status_code == 200
        assert updated.json()["name"] == "Ana Updated"

        assert client.delete(f"/users/{user_id}").status_code == 204
        assert client.get(f"/users/{user_id}").status_code == 404


def test_user_validation_rejects_invalid_email():
    with TestClient(app) as client:
        response = client.post("/users", json={"name": "Invalid", "email": "not-an-email"})
        assert response.status_code == 422
