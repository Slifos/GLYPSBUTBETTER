export interface DailyRegistration{
    periodStart: string
    registrations: number
}

export interface WeeklyRegistration{
    periodStart: string
    registrations: number
}

export interface EventStatistics{
    eventId: number
    remainingPlaces: number
    occupancyRate: number
    full: boolean
    cancellationRate: number
    attendedParticipants: number
    attendanceRate: number
    dailyRegistrations: DailyRegistration[]
    weeklyRegistrations: WeeklyRegistration[]
    registrationsPerDay: number
    estimatedFullAt: string | null
    alerts: Record<string, unknown>
}

export interface DashboardComparison{
    eventId: number
    eventType: string
    currentParticipants: number
    occupancyRate: number
    cancellationRate: number
    attendanceRate: number
    registrationsPerDay: number
}

export interface PopularEvent{
    eventId: number
    eventType: string
    currentParticipants: number
    occupancyRate: number
    rank: number
}

export interface StatisticsByType{
    eventType: string
    eventCount: number
    averageParticipants: number
    averageAttendanceRate: number
    recommendedCapacity: number
}

export interface GlobalStatistics{
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

export interface DashboardStatistics{
    comparisons: DashboardComparison[]
    popularEvents: PopularEvent[]
    statisticsByType: StatisticsByType[]
    global: GlobalStatistics
}