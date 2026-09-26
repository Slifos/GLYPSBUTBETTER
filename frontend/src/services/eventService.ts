import api  from "./api";

export interface EventCreateRequest {
  title: string
  eventType: string
  description?: string
  location?: string
  startDate: string
  maxParticipants: number
}

export interface EventUpdateRequest {
  title: string
  eventType: string
  description?: string
  location?: string
  startDate: string
  maxParticipants: number
}

export interface EventResponse {
  id: number
  title: string
  eventType: string
  description: string
  location: string
  startDate: string
  maxParticipants: number
  currentParticipants: number
  remainingPlaces: number
  createdAt: string
}

export const eventService = {
    async getEvents(): Promise<EventResponse[]> {
        const response = await api.get<EventResponse[]>('/events');
        return response.data;
    },

    async getEventById(id: number): Promise<EventResponse> {
        const response = await api.get<EventResponse>(`/events/${id}`);
        return response.data;
    },

    async createEvent(event: EventCreateRequest): Promise<EventResponse> {
        const response = await api.post<EventResponse>('/events', event);
        return response.data;
    },
    
    async updateEvent(id: number, event: EventUpdateRequest): Promise<EventResponse> {
        const response = await api.put<EventResponse>(`/events/${id}`, event);
        return response.data;
    },

    async deleteEvent(id: number): Promise<void> {
        await api.delete(`/events/${id}`);
    },

}