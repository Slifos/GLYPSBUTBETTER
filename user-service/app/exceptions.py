class UserNotFoundError(Exception):
    """Raised when a requested user does not exist."""


class EmailAlreadyUsedError(Exception):
    """Raised when an email is already associated with another user."""
