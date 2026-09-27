import os

from sqlalchemy import create_engine
from sqlalchemy.orm import Session

# SQLite keeps the service easy to run and test locally; Compose provides PostgreSQL.
engine = create_engine(os.getenv("DATABASE_URL", "sqlite:///users.db"))


def get_session():
    with Session(engine) as session:
        yield session
