"""Compatibility entrypoint used by Uvicorn and Docker."""

from app.main import app

__all__ = ["app"]
