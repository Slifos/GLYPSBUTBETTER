export interface EventComparison {
  eventId: number
  eventType: string
  currentParticipants: number
  occupancyRate: number
  cancellationRate: number
  attendanceRate: number
  registrationsPerDay: number
}

export interface PopularEvent {
  eventId: number
  eventType: string
  currentParticipants: number
  occupancyRate: number
  rank: number
}

export interface EventTypeStatistics {
  eventType: string
  eventCount: number
  averageParticipants: number
  averageAttendanceRate: number
  recommendedCapacity: number
}

export interface GlobalStatistics {
  totalEvents: number
  fullEvents: number
  totalCapacity: number
  totalCurrentParticipants: number
  totalCancelledParticipants: number
  totalAttendedParticipants: number
  averageParticipantsPerEvent: number
  overallOccupancyRate: number
  overallCancellationRate: number
  overallAttendanceRate: number
}

export interface DashboardStatistics {
  comparisons: EventComparison[]
  popularEvents: PopularEvent[]
  statisticsByType: EventTypeStatistics[]
  global: GlobalStatistics
}
