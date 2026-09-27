import api from './api'
import type{EventStatistics, DashboardStatistics} from '../types/statistics'

export const statisticsService = {
    async getEventStatistics(eventId: number): Promise<EventStatistics> {
        const response = await api.get(`/events/${eventId}/statistics`)
        return response.data
    },

    async getDashboardStatistics(): Promise<DashboardStatistics> {
        const response = await api.get<DashboardStatistics>('/events/statistics/dashboard')
        return response.data
    }
}