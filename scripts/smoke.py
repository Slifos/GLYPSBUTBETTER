"""Exercise the running Compose stack through its public gateway."""

import json
import sys
import time
import uuid
from datetime import datetime, timedelta, timezone
from urllib.error import HTTPError
from urllib.request import Request, urlopen


base_url = sys.argv[1].rstrip("/") if len(sys.argv) > 1 else "http://localhost:8083"


def request(method, path, body=None):
    data = None if body is None else json.dumps(body).encode()
    req = Request(
        base_url + path,
        data=data,
        method=method,
        headers={"Content-Type": "application/json"},
    )
    try:
        with urlopen(req, timeout=10) as response:
            raw = response.read()
            return json.loads(raw) if raw and response.headers.get_content_type() == "application/json" else raw
    except HTTPError as exc:
        raise AssertionError(f"{method} {path}: {exc.code} {exc.read().decode()}") from exc


run_id = uuid.uuid4().hex[:12]
ready_by = time.monotonic() + 90
while True:
    try:
        request("GET", "/api/events")
        request("GET", "/api/users")
        request("GET", "/api/notifications/users/0")
        break
    except Exception:
        if time.monotonic() >= ready_by:
            raise AssertionError("Services did not become ready within 90 seconds")
        time.sleep(2)

assert b"GLYPSBUTBETTER" in request("GET", "/")
first = request("POST", "/api/users", {"name": "Smoke One", "email": f"smoke-{run_id}-1@example.com"})
second = request("POST", "/api/users", {"name": "Smoke Two", "email": f"smoke-{run_id}-2@example.com"})
event = request("POST", "/api/events", {
    "title": f"Smoke event {run_id}",
    "eventType": "TEST",
    "description": "Integration smoke test",
    "location": "Test room",
    "startDate": (datetime.now(timezone.utc) + timedelta(days=1)).replace(tzinfo=None).isoformat(timespec="seconds"),
    "maxParticipants": 2,
})
event_id = event["id"]
for user in (first, second):
    request("POST", f"/api/events/{event_id}/registrations", {"userId": user["id"]})

details = request("GET", f"/api/events/{event_id}")
assert details["currentParticipants"] == 2 and details["remainingPlaces"] == 0
analytics = request("GET", f"/api/events/{event_id}/statistics")
assert analytics["full"] is True
dashboard = request("GET", "/api/events/statistics/dashboard")
assert dashboard["global"]["totalEvents"] >= 1

deadline = time.monotonic() + 30
while True:
    notifications = [
        request("GET", f"/api/notifications/users/{user['id']}") for user in (first, second)
    ]
    if all(
        {item["kind"] for item in records if item["event_id"] == event_id}
        >= {"REGISTRATION_CONFIRMED", "EVENT_FULL"}
        for records in notifications
    ):
        break
    if time.monotonic() >= deadline:
        raise AssertionError("Registration and full-event notifications did not arrive within 30 seconds")
    time.sleep(1)

request("DELETE", f"/api/events/{event_id}/registrations/{first['id']}")
after_cancel = request("GET", f"/api/events/{event_id}")
assert after_cancel["currentParticipants"] == 1 and after_cancel["remainingPlaces"] == 1
print(f"Smoke test passed: event {event_id}, users {first['id']} and {second['id']}")
