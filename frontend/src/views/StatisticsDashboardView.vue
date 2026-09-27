<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { statisticsService } from '../services/statisticsService'
import type { DashboardStatistics } from '../types/statistics'

const statistics = ref<DashboardStatistics | null>(null)
const loading = ref(true)
const error = ref('')

const loadStatistics = async () => {
    try {
        loading.value = true
        error.value = ''

        statistics.value =await statisticsService.getDashboardStatistics()
    } catch (err) {
        console.error(err)
        error.value ='Impossible de charger les statistiques.'
    } finally {
        loading.value = false
    }
}

const formatPercentage = (value: number) => {return `${value.toFixed(1)} %`}

onMounted(loadStatistics)
</script>

<template>
    <h1>Tableau de bord</h1>

    <p v-if="loading">Chargement des statistiques...</p>

    <p v-else-if="error">{{ error }}</p>

    <div v-else-if="statistics">

      <section>
        <h2>Statistiques globales</h2>

        <p><strong>Total des événements :</strong>{{ statistics.global.totalEvents }}</p>

        <p><strong>Événements complets :</strong>{{ statistics.global.fullEvents }}</p>

        <p><strong>Capacité totale :</strong>{{ statistics.global.totalCapacity }}</p>

        <p><strong>Participants actuels :</strong>{{ statistics.global.totalCurrentParticipants }}</p>

        <p><strong>Participants annulés :</strong>{{ statistics.global.totalCancelledParticipants }}</p>

        <p><strong>Participants présents :</strong>{{ statistics.global.totalAttendedParticipants }}</p>

        <p><strong>Moyenne de participants par événement :</strong>{{ statistics.global.averageParticipantsPerEvent.toFixed(1) }}</p>

        <p><strong>Taux global de remplissage :</strong>{{ formatPercentage(statistics.global.overallOccupancyRate) }}</p>

        <p><strong>Taux global d'annulation :</strong>{{ formatPercentage(statistics.global.overallCancellationRate) }}</p>

        <p><strong>Taux global de présence :</strong>{{ formatPercentage(statistics.global.overallAttendanceRate) }}</p>
      </section>

      <hr />

      <section>
        <h2>Événements populaires</h2>

        <div v-for="event in statistics.popularEvents" :key="event.eventId">
          <h3>#{{ event.rank }} — Événement {{ event.eventId }}</h3>

          <p><strong>Type :</strong>{{ event.eventType }}</p>

          <p><strong>Participants :</strong>{{ event.currentParticipants }}</p>

          <p><strong>Remplissage :</strong>{{ formatPercentage(event.occupancyRate) }}</p>
        </div>
      </section>

      <hr />

      <section>
        <h2>Comparaison des événements</h2>

        <div v-for="event in statistics.comparisons" :key="event.eventId">
          <h3>Événement {{ event.eventId }}</h3>

          <p><strong>Type :</strong>{{ event.eventType }}</p>

          <p><strong>Participants :</strong>{{ event.currentParticipants }}</p>

          <p><strong>Remplissage :</strong>{{ formatPercentage(event.occupancyRate) }}</p>

          <p><strong>Annulation :</strong>{{ formatPercentage(event.cancellationRate) }}</p>

          <p><strong>Présence :</strong>{{ formatPercentage(event.attendanceRate) }}</p>

          <p><strong>Inscriptions par jour :</strong>{{ event.registrationsPerDay }}</p>
        </div>
      </section>

      <hr />

      <section>
        <h2>Statistiques par type</h2>

        <div v-for="type in statistics.statisticsByType" :key="type.eventType">
          <h3>{{ type.eventType }}</h3>

          <p><strong>Nombre d'événements :</strong>{{ type.eventCount }}</p>

          <p><strong>Moyenne de participants :</strong>{{ type.averageParticipants.toFixed(1) }}</p>

          <p><strong>Taux moyen de présence :</strong>{{ formatPercentage(type.averageAttendanceRate) }}</p>

          <p><strong>Capacité recommandée :</strong>{{ type.recommendedCapacity }}</p>
        </div>
      </section>

    </div>
</template>