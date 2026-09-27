import logging

from sqlalchemy.exc import IntegrityError

from .exceptions import EmailAlreadyUsedError, UserNotFoundError
from .models import User
from .repositories import UserRepository
from .schemas import UserIn

logger = logging.getLogger(__name__)


class UserService:
    """Stateless user business operations backed by a repository."""

    def __init__(self, repository: UserRepository):
        self.repository = repository

    def create(self, body: UserIn) -> User:
        user = self.repository.add(User(**body.model_dump()))
        try:
            self.repository.commit()
        except IntegrityError as exc:
            self.repository.rollback()
            raise EmailAlreadyUsedError from exc
        logger.info("Created user id=%s", user.id)
        return user

    def get(self, user_id: int) -> User:
        user = self.repository.get(user_id)
        if user is None:
            raise UserNotFoundError
        return user

    def list(self) -> list[User]:
        return self.repository.list()

    def update(self, user_id: int, body: UserIn) -> User:
        user = self.get(user_id)
        user.name = body.name
        user.email = body.email
        try:
            self.repository.commit()
        except IntegrityError as exc:
            self.repository.rollback()
            raise EmailAlreadyUsedError from exc
        logger.info("Updated user id=%s", user_id)
        return user

    def delete(self, user_id: int) -> None:
        self.repository.delete(self.get(user_id))
        self.repository.commit()
        logger.info("Deleted user id=%s", user_id)
