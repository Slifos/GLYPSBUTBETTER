import os

from fastapi import Depends, FastAPI, HTTPException
from pydantic import BaseModel, EmailStr
from sqlalchemy import create_engine, select
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import DeclarativeBase, Mapped, Session, mapped_column

# sqlite fallback for local runs/tests, compose sets postgres
engine = create_engine(os.getenv("DATABASE_URL", "sqlite:///users.db"))


class Base(DeclarativeBase):
    pass


class User(Base):
    __tablename__ = "users"
    id: Mapped[int] = mapped_column(primary_key=True)
    name: Mapped[str]
    email: Mapped[str] = mapped_column(unique=True)


class UserIn(BaseModel):
    name: str
    email: EmailStr


class UserOut(UserIn):
    id: int
    model_config = {"from_attributes": True}


# Create the initial schema for this small service.
Base.metadata.create_all(engine)
app = FastAPI()


def db():
    with Session(engine) as s:
        yield s


@app.post("/users", response_model=UserOut, status_code=201)
def create_user(body: UserIn, s: Session = Depends(db)):
    user = User(**body.model_dump())
    s.add(user)
    try:
        s.commit()
    except IntegrityError:
        raise HTTPException(400, "EMAIL_ALREADY_USED")
    return user


@app.get("/users", response_model=list[UserOut])
def list_users(s: Session = Depends(db)):
    return s.scalars(select(User)).all()


@app.get("/users/{user_id}", response_model=UserOut)
def get_user(user_id: int, s: Session = Depends(db)):
    user = s.get(User, user_id)
    if not user:
        raise HTTPException(404, "USER_NOT_FOUND")
    return user


@app.put("/users/{user_id}", response_model=UserOut)
def update_user(user_id: int, body: UserIn, s: Session = Depends(db)):
    user = s.get(User, user_id)
    if not user:
        raise HTTPException(404, "USER_NOT_FOUND")
    user.name = body.name
    user.email = body.email
    try:
        s.commit()
    except IntegrityError:
        s.rollback()
        raise HTTPException(400, "EMAIL_ALREADY_USED")
    return user


@app.delete("/users/{user_id}", status_code=204)
def delete_user(user_id: int, s: Session = Depends(db)):
    user = s.get(User, user_id)
    if not user:
        raise HTTPException(404, "USER_NOT_FOUND")
    s.delete(user)
    s.commit()
