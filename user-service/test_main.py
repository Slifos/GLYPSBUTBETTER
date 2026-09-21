import os
import tempfile

os.environ["DATABASE_URL"] = f"sqlite:///{tempfile.mkdtemp()}/t.db"

from fastapi.testclient import TestClient

import main

c = TestClient(main.app)


def test_flow():
    r = c.post("/users", json={"name": "Ana", "email": "ana@efrei.net"})
    assert r.status_code == 201
    uid = r.json()["id"]
    assert c.get(f"/users/{uid}").json()["name"] == "Ana"
    assert len(c.get("/users").json()) == 1
    assert c.post("/users", json={"name": "B", "email": "ana@efrei.net"}).json()["detail"] == "EMAIL_ALREADY_USED"
    assert c.get("/users/999").json()["detail"] == "USER_NOT_FOUND"
