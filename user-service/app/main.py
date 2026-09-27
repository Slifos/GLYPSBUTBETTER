from contextlib import asynccontextmanager

from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse

from .database import engine
from .exceptions import EmailAlreadyUsedError, UserNotFoundError
from .models import Base
from .web import router


@asynccontextmanager
async def lifespan(_app: FastAPI):
    Base.metadata.create_all(engine)
    yield


app = FastAPI(lifespan=lifespan)
app.include_router(router)


@app.exception_handler(UserNotFoundError)
async def user_not_found(_request: Request, _exception: UserNotFoundError):
    return JSONResponse(status_code=404, content={"detail": "USER_NOT_FOUND"})


@app.exception_handler(EmailAlreadyUsedError)
async def email_already_used(_request: Request, _exception: EmailAlreadyUsedError):
    return JSONResponse(status_code=400, content={"detail": "EMAIL_ALREADY_USED"})
