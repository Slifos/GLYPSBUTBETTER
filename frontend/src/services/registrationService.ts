import api from "./api";
import type { RegistrationResponse } from "../types/registration";

export const registrationService = {
    async registerForEvent(eventId: number, userId: number): Promise<RegistrationResponse> {
        const response = await api.post<RegistrationResponse>(`/events/${eventId}/registrations`, { userId });
        return response.data;
    },
    
    async getRegistrations(eventId: number): Promise<RegistrationResponse[]> {
        const response = await api.get<RegistrationResponse[]>(`/events/${eventId}/registrations`);
        return response.data;
    },

    async cancelRegistration(eventId: number, userId: number): Promise<void> {
        await api.delete(`/events/${eventId}/registrations/${userId}`);
    },

    async markAttended(eventId: number, userId: number): Promise<RegistrationResponse> {
        const response = await api.post<RegistrationResponse>(`/events/${eventId}/registrations/${userId}/attendance`);
        return response.data;
    },
}