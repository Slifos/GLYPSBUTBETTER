import api from './api'
import type { DashboardStatistics } from '../types/statistics'

export const statisticsService = {
  async getDashboard(): Promise<DashboardStatistics> {
    const response = await api.get<DashboardStatistics>('/events/statistics/dashboard')
    return response.data
  },
}
