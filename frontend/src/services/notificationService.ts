import api from './api'

export interface Notification {
  id: number
  user_id: number
  event_id: number
  kind: string
  message: string
  created_at: string
}

export const notificationService = {
  async forUser(userId: number): Promise<Notification[]> {
    const response = await api.get<Notification[]>(`/notifications/users/${userId}`)
    return response.data
  },
}
