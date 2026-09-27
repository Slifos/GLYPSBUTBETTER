from sqlalchemy import select
from sqlalchemy.orm import Session

from .models import User


class UserRepository:
    """Database access only; business decisions belong to UserService."""

    def __init__(self, session: Session):
        self.session = session

    def add(self, user: User) -> User:
        self.session.add(user)
        return user

    def get(self, user_id: int) -> User | None:
        return self.session.get(User, user_id)

    def list(self) -> list[User]:
        return list(self.session.scalars(select(User)).all())

    def delete(self, user: User) -> None:
        self.session.delete(user)

    def commit(self) -> None:
        self.session.commit()

    def rollback(self) -> None:
        self.session.rollback()
