from pydantic import BaseModel, EmailStr


class UserIn(BaseModel):
    name: str
    email: EmailStr


class UserOut(UserIn):
    id: int
    model_config = {"from_attributes": True}
