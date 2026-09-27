export interface UserCreateRequest{
    name: string
    email: string
}

export interface UserResponse {
    id: number
    name: string
    email: string
}