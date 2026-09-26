import api from "./api";

export interface UserCreateRequest{
    name: string
    email: string
}

export interface UserResponse {
    id: number
    name: string
    email: string
}

export const userService = {
    async getUsers(): Promise<UserResponse[]> {
        const response = await api.get<UserResponse[]>('/users');
        return response.data;
    },

    async getUserById(id: number): Promise<UserResponse> {
        const response = await api.get<UserResponse>(`/users/${id}`);
        return response.data;
    },

    async createUser(user: UserCreateRequest): Promise<UserResponse> {
        const response = await api.post<UserResponse>('/users', user);
        return response.data;
    },
}