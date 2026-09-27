from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from .database import get_session
from .repositories import UserRepository
from .schemas import UserIn, UserOut
from .services import UserService

router = APIRouter(prefix="/users", tags=["users"])


def get_user_service(session: Session = Depends(get_session)) -> UserService:
    return UserService(UserRepository(session))


@router.post("", response_model=UserOut, status_code=201)
def create_user(body: UserIn, service: UserService = Depends(get_user_service)):
    return service.create(body)


@router.get("", response_model=list[UserOut])
def list_users(service: UserService = Depends(get_user_service)):
    return service.list()


@router.get("/{user_id}", response_model=UserOut)
def get_user(user_id: int, service: UserService = Depends(get_user_service)):
    return service.get(user_id)


@router.put("/{user_id}", response_model=UserOut)
def update_user(user_id: int, body: UserIn, service: UserService = Depends(get_user_service)):
    return service.update(user_id, body)


@router.delete("/{user_id}", status_code=204)
def delete_user(user_id: int, service: UserService = Depends(get_user_service)):
    service.delete(user_id)
