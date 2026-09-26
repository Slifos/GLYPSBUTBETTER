import api from "./api";

export interface RegistrationResponse {
  id: number
  eventId: number
  userId: number
  status: string
  createdAt: string
  updatedAt: string
}

export const registrationService = {
    async registerForEvent(eventId: number, userId: number): Promise<RegistrationResponse> {
        const response = await api.post<RegistrationResponse>(`/events/${eventId}/register`, { userId });
        return response.data;
    },
    
    async getRegistrations(eventd: number): Promise<RegistrationResponse[]> {
        const response = await api.get<RegistrationResponse[]>(`/events/${eventd}/registrations`);
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