import json
import logging
import os
import threading
from contextlib import asynccontextmanager
from datetime import datetime, timezone

import pika
from fastapi import Depends, FastAPI
from pydantic import BaseModel
from sqlalchemy import DateTime, String, create_engine, select
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import DeclarativeBase, Mapped, Session, mapped_column

logger = logging.getLogger(__name__)
DATABASE_URL = os.getenv("DATABASE_URL", "sqlite:///notifications.db")
RABBITMQ_URL = os.getenv("RABBITMQ_URL", "amqp://guest:guest@localhost:5672/")
EXCHANGE = "campusconnect.notifications"
QUEUE = "notification-service"
ROUTING_KEY = "notification"

engine = create_engine(DATABASE_URL)


class Base(DeclarativeBase):
    pass


class Notification(Base):
    __tablename__ = "notifications"

    id: Mapped[int] = mapped_column(primary_key=True)
    message_id: Mapped[str] = mapped_column(String(36), unique=True)
    user_id: Mapped[int]
    event_id: Mapped[int]
    kind: Mapped[str] = mapped_column(String(32))
    message: Mapped[str]
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), default=lambda: datetime.now(timezone.utc))


class NotificationOut(BaseModel):
    id: int
    user_id: int
    event_id: int
    kind: str
    message: str
    created_at: datetime
    model_config = {"from_attributes": True}


def db():
    with Session(engine) as session:
        yield session


def store_notification(payload: dict) -> None:
    required = ("messageId", "userId", "eventId", "kind", "message")
    if any(key not in payload for key in required):
        raise ValueError("Notification message is missing required fields")
    with Session(engine) as session:
        session.add(Notification(
            message_id=str(payload["messageId"]),
            user_id=int(payload["userId"]),
            event_id=int(payload["eventId"]),
            kind=str(payload["kind"]),
            message=str(payload["message"]),
        ))
        try:
            session.commit()
        except IntegrityError:
            session.rollback()  # Redelivery of an already stored message.


def consume(stop: threading.Event) -> None:
    while not stop.is_set():
        try:
            connection = pika.BlockingConnection(pika.URLParameters(RABBITMQ_URL))
            channel = connection.channel()
            channel.exchange_declare(exchange=EXCHANGE, exchange_type="direct", durable=True)
            channel.queue_declare(queue=QUEUE, durable=True)
            channel.queue_bind(queue=QUEUE, exchange=EXCHANGE, routing_key=ROUTING_KEY)
            channel.basic_qos(prefetch_count=10)

            def handle(ch, method, _properties, body):
                try:
                    store_notification(json.loads(body))
                except (ValueError, TypeError, json.JSONDecodeError):
                    logger.exception("Discarding invalid notification message")
                    ch.basic_reject(method.delivery_tag, requeue=False)
                except Exception:
                    logger.exception("Could not store notification; will retry")
                    ch.basic_nack(method.delivery_tag, requeue=True)
                else:
                    ch.basic_ack(method.delivery_tag)

            channel.basic_consume(queue=QUEUE, on_message_callback=handle)
            while not stop.is_set():
                connection.process_data_events(time_limit=1)
            connection.close()
        except Exception:
            logger.exception("Notification broker unavailable; reconnecting")
            stop.wait(3)


@asynccontextmanager
async def lifespan(_app: FastAPI):
    Base.metadata.create_all(engine)
    stop = threading.Event()
    if os.getenv("NOTIFICATION_CONSUMER_ENABLED", "true").lower() == "true":
        thread = threading.Thread(target=consume, args=(stop,), daemon=True)
        thread.start()
    try:
        yield
    finally:
        stop.set()


app = FastAPI(lifespan=lifespan)


@app.get("/health")
def health():
    return {"status": "ok"}


@app.get("/notifications/users/{user_id}", response_model=list[NotificationOut])
def list_notifications(user_id: int, session: Session = Depends(db)):
    return session.scalars(
        select(Notification).where(Notification.user_id == user_id).order_by(Notification.id.desc())
    ).all()
