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