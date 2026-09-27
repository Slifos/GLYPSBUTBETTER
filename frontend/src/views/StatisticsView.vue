<script setup lang="ts">
import { computed, onMounted, ref } from 'vue'
import { eventService } from '../services/eventService'
import { statisticsService } from '../services/statisticsService'
import type { EventResponse } from '../types/event'
import type { DashboardStatistics } from '../types/statistics'

const dashboard = ref<DashboardStatistics | null>(null)
const events = ref<EventResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)
const updatedAt = ref<Date | null>(null)

const eventNames = computed(() => new Map(events.value.map((event) => [event.id, event.title])))
const maxTypeCount = computed(() => Math.max(1, ...dashboard.value?.statisticsByType.map((type) => type.eventCount) ?? [1]))
const comparisons = computed(() =>
  [...(dashboard.value?.comparisons ?? [])].sort((a, b) => b.occupancyRate - a.occupancyRate),
)

const eventName = (id: number) => eventNames.value.get(id) ?? `Événement #${id}`
const number = (value: number) => new Intl.NumberFormat('fr-FR', { maximumFractionDigits: 1 }).format(value)
const percent = (value: number) => `${number(value)} %`
const width = (value: number) => `${Math.min(100, Math.max(0, value))}%`

const loadDashboard = async () => {
  loading.value = true
  error.value = null
  try {
    const [statistics, eventList] = await Promise.all([
      statisticsService.getDashboard(),
      eventService.getEvents(),
    ])
    dashboard.value = statistics
    events.value = eventList
    updatedAt.value = new Date()
  } catch (err) {
    console.error('Erreur lors du chargement des statistiques :', err)
    error.value = 'Impossible de charger les statistiques. Réessaie dans quelques instants.'
  } finally {
    loading.value = false
  }
}

onMounted(loadDashboard)
</script>

<template>
  <main class="page container statistics-page">
    <div class="statistics-header">
      <div class="page-heading">
        <span class="eyebrow">Vue d'ensemble</span>
        <h1>Statistiques<span class="heading-dot">.</span></h1>
        <p>Activité des événements et des inscriptions.</p>
      </div>
      <div class="statistics-header__actions">
        <span v-if="updatedAt" class="muted">Actualisé à {{ updatedAt.toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit' }) }}</span>
        <button class="button button--outline" type="button" :disabled="loading" @click="loadDashboard">{{ loading ? 'Chargement...' : 'Actualiser' }}</button>
      </div>
    </div>

    <p v-if="loading && !dashboard" class="notice" role="status">Chargement des statistiques...</p>
    <p v-if="error" class="notice notice--error" role="alert">{{ error }}</p>

    <template v-if="dashboard">
      <section class="metric-grid" aria-label="Indicateurs globaux">
        <article class="metric-card surface"><span>Événements</span><strong>{{ number(dashboard.global.totalEvents) }}</strong><small>{{ number(dashboard.global.fullEvents) }} complet{{ dashboard.global.fullEvents > 1 ? 's' : '' }}</small></article>
        <article class="metric-card surface"><span>Participants inscrits</span><strong>{{ number(dashboard.global.totalCurrentParticipants) }}</strong><small>sur {{ number(dashboard.global.totalCapacity) }} places</small></article>
        <article class="metric-card surface"><span>Taux d'occupation</span><strong>{{ percent(dashboard.global.overallOccupancyRate) }}</strong><div class="metric-track"><span :style="{ width: width(dashboard.global.overallOccupancyRate) }"></span></div></article>
        <article class="metric-card surface"><span>Présences confirmées</span><strong>{{ number(dashboard.global.totalAttendedParticipants) }}</strong><small>{{ percent(dashboard.global.overallAttendanceRate) }} des inscriptions</small></article>
      </section>

      <div v-if="dashboard.global.totalEvents === 0" class="empty-state surface">
        <h2>Aucune donnée disponible</h2>
        <p>Les statistiques apparaîtront après la création d'un événement.</p>
        <RouterLink class="button" to="/events/create">Créer un événement</RouterLink>
      </div>
      <template v-else>
        <div class="statistics-grid">
          <section class="panel surface" aria-labelledby="popular-heading">
            <div class="panel__heading"><div><span class="eyebrow">Classement</span><h2 id="popular-heading">Événements populaires</h2></div><span class="panel__note">Par inscriptions</span></div>
            <p v-if="dashboard.popularEvents.length === 0" class="muted">Aucun événement à afficher.</p>
            <ol v-else class="popular-list">
              <li v-for="event in dashboard.popularEvents.slice(0, 5)" :key="event.eventId">
                <span class="popular-list__rank">{{ event.rank.toString().padStart(2, '0') }}</span>
                <div class="popular-list__name"><RouterLink :to="{ name: 'event-details', params: { id: event.eventId } }">{{ eventName(event.eventId) }}</RouterLink><small>{{ event.eventType }}</small></div>
                <div class="popular-list__value"><strong>{{ event.currentParticipants }}</strong><small>{{ percent(event.occupancyRate) }} occupé</small></div>
              </li>
            </ol>
          </section>

          <section class="panel surface" aria-labelledby="types-heading">
            <div class="panel__heading"><div><span class="eyebrow">Répartition</span><h2 id="types-heading">Par catégorie</h2></div><span class="panel__note">Nombre d'événements</span></div>
            <p v-if="dashboard.statisticsByType.length === 0" class="muted">Aucune catégorie à afficher.</p>
            <div v-else class="type-list">
              <div v-for="type in dashboard.statisticsByType" :key="type.eventType" class="type-row">
                <div class="type-row__label"><span>{{ type.eventType }}</span><strong>{{ type.eventCount }}</strong></div>
                <div class="type-row__track"><span :style="{ width: width(type.eventCount / maxTypeCount * 100) }"></span></div>
                <small>{{ number(type.averageParticipants) }} participant{{ type.averageParticipants > 1 ? 's' : '' }} en moyenne</small>
              </div>
            </div>
          </section>
        </div>

        <section class="panel surface comparison-panel" aria-labelledby="comparison-heading">
          <div class="panel__heading"><div><span class="eyebrow">Détail</span><h2 id="comparison-heading">Comparaison des événements</h2></div><span class="panel__note">{{ comparisons.length }} événements</span></div>
          <div class="table-scroll">
            <table>
              <thead><tr><th scope="col">Événement</th><th scope="col">Catégorie</th><th scope="col">Participants</th><th scope="col">Occupation</th><th scope="col">Annulations</th><th scope="col">Présence</th></tr></thead>
              <tbody>
                <tr v-for="event in comparisons" :key="event.eventId">
                  <th scope="row"><RouterLink :to="{ name: 'event-details', params: { id: event.eventId } }">{{ eventName(event.eventId) }}</RouterLink></th>
                  <td>{{ event.eventType }}</td><td>{{ number(event.currentParticipants) }}</td><td><span class="occupancy"><span class="occupancy__track"><span :style="{ width: width(event.occupancyRate) }"></span></span>{{ percent(event.occupancyRate) }}</span></td><td>{{ percent(event.cancellationRate) }}</td><td>{{ percent(event.attendanceRate) }}</td>
                </tr>
              </tbody>
            </table>
          </div>
        </section>
      </template>
    </template>
  </main>
</template>

<style scoped>
.statistics-header{display:flex;align-items:end;justify-content:space-between;gap:20px}.statistics-header .page-heading{margin-bottom:30px}.statistics-header__actions{display:flex;align-items:center;gap:16px;margin-bottom:30px;font-size:12px;white-space:nowrap}.heading-dot{color:var(--orange)}.metric-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:18px;margin-bottom:28px}.metric-card{display:flex;flex-direction:column;min-height:160px;padding:23px}.metric-card>span{color:var(--muted);font-size:13px;font-weight:700}.metric-card strong{margin:16px 0 4px;font-family:'Space Grotesk';font-size:38px;letter-spacing:-.06em;line-height:1}.metric-card small{margin-top:auto;color:var(--muted);font-size:12px}.metric-track,.type-row__track,.occupancy__track{height:8px;overflow:hidden;border-radius:10px;background:var(--green-light)}.metric-track{margin-top:auto}.metric-track>span,.type-row__track>span,.occupancy__track>span{display:block;height:100%;border-radius:10px;background:var(--orange)}.statistics-grid{display:grid;grid-template-columns:1fr 1fr;gap:22px;margin-bottom:22px}.panel{min-width:0;padding:27px}.panel__heading{display:flex;align-items:end;justify-content:space-between;gap:12px;margin-bottom:22px}.panel h2{margin:7px 0 0;font-size:24px}.panel__note{color:var(--muted);font-size:11px;white-space:nowrap}.popular-list{list-style:none;margin:0;padding:0}.popular-list li{display:flex;align-items:center;gap:14px;padding:15px 0;border-top:1px solid var(--line)}.popular-list__rank{color:var(--orange);font-family:'Space Grotesk';font-size:24px;font-weight:700}.popular-list__name{flex:1;min-width:0}.popular-list__name a{display:block;overflow:hidden;font-size:14px;font-weight:800;text-overflow:ellipsis;white-space:nowrap}.popular-list__name a:hover,.comparison-panel a:hover{color:var(--orange)}.popular-list__name small,.popular-list__value small{display:block;margin-top:4px;color:var(--muted);font-size:11px}.popular-list__value{text-align:right;white-space:nowrap}.popular-list__value strong{font-family:'Space Grotesk';font-size:18px}.type-list{display:grid;gap:19px}.type-row__label{display:flex;justify-content:space-between;gap:10px;margin-bottom:8px;font-size:13px}.type-row__track>span{background:var(--green)}.type-row small{display:block;margin-top:5px;color:var(--muted);font-size:11px}.comparison-panel{margin-bottom:25px}.table-scroll{overflow-x:auto}table{width:100%;border-collapse:collapse;text-align:left;font-size:13px;white-space:nowrap}thead{background:#f3f6ef}th,td{padding:14px 12px;border-bottom:1px solid var(--line)}thead th{color:var(--muted);font-size:11px;font-weight:800;text-transform:uppercase;letter-spacing:.04em}tbody th{font-weight:700}td{color:#435b52}.occupancy{display:flex;align-items:center;gap:9px}.occupancy__track{width:65px;height:6px;flex:none}.empty-state{padding:45px;text-align:center}.empty-state p{color:var(--muted)}
@media(max-width:900px){.metric-grid{grid-template-columns:repeat(2,minmax(0,1fr))}.statistics-grid{grid-template-columns:1fr}}@media(max-width:600px){.statistics-header{display:block}.statistics-header__actions{justify-content:space-between;margin-top:-12px}.metric-grid{gap:10px}.metric-card{min-height:130px;padding:17px}.metric-card strong{font-size:30px}.panel{padding:20px}.panel h2{font-size:21px}.panel__note{display:none}}
</style>
