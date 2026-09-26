<script setup lang="ts">
import { eventService} from '../services/eventService'
import { ref, onMounted } from 'vue'
import {type EventResponse} from '../types/event'

const events = ref<EventResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)

onMounted(async () => {
    try {
        events.value = await eventService.getEvents()
    } catch (err) {
        console.error('Erreur lors de la récupération des événements:', err)
        error.value = 'Impossible de récupérer les événements. Veuillez réessayer plus tard.'
    } finally {
        loading.value = false
    }
})
</script>

<template>
    <h1>GLYPSBUTBETTER</h1>
    <h2>Liste des événements</h2>

    <p v-if="loading">Chargement des événements...</p>
    <p v-else-if="error">{{ error }}</p>
    <p v-else-if="events.length === 0">Aucun événement disponible.</p>

    <div v-else>
            <article v-for="event in events" :key="event.id">
                <h3>{{ event.title }}</h3>
                <p>{{ event.description }}</p>
                <p>Date: {{ event.startDate }}</p>
                <p>Lieu: {{ event.location }}</p>
                <p class="remaining-places"
                :class="{
                    'full': event.remainingPlaces === 0,
                    'few-places': event.remainingPlaces > 0 && event.remainingPlaces <= 10
                }"
                >
                {{ event.remainingPlaces }} places restantes
                </p>
                <p>{{ event.currentParticipants }} / {{ event.maxParticipants }} participants</p>
            </article>
    </div>
</template>

<style scoped>
h1 {
  text-align: center;
  font-size: 2.2rem;
  font-weight: 800;
  letter-spacing: 0.05em;
  background: linear-gradient(135deg, #1e3a8a, #3b82f6);
  -webkit-background-clip: text;
  background-clip: text;
  color: transparent;
  margin-bottom: 0.25rem;
}

h2 {
  text-align: center;
  font-size: 1.1rem;
  font-weight: 500;
  color: #64748b;
  margin-bottom: 2rem;
}

/* États : chargement / erreur / vide */
p {
  text-align: center;
  font-size: 1rem;
  color: #475569;
  padding: 1rem;
}

p[v-if="error"],
.error {
  color: #b91c1c;
  background: #fee2e2;
  border-radius: 8px;
}

/* Grille des événements */
div {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 1.5rem;
  max-width: 1100px;
  margin: 0 auto;
  padding: 1rem;
}

article {
  background: #ffffff;
  border: 1px solid #dbeafe;
  border-radius: 14px;
  padding: 1.25rem 1.5rem;
  box-shadow: 0 2px 6px rgba(30, 58, 138, 0.06);
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

article:hover {
  transform: translateY(-3px);
  box-shadow: 0 8px 20px rgba(30, 58, 138, 0.12);
  border-color: #93c5fd;
}

article h3 {
    text-align: center;
    color: #1e40af;
    font-size: 1.2rem;
    margin-bottom: 0.5rem;
}

article p {
  text-align: left;
  padding: 0;
  font-size: 0.92rem;
  color: #334155;
  margin: 0.3rem 0;
}

/* Places restantes en évidence */
.remaining-places {
  display: inline-block;
  background: #dbeafe;
  color: #1d4ed8;
  font-weight: 600;
  padding: 0.2rem 0.6rem;
  border-radius: 20px;
  font-size: 0.8rem;
  margin-top: 0.5rem;
}

.remaining-places.few-places {
  background: #fef3c7;
  color: #b45309;
}

.remaining-places.full {
  background: #fee2e2;
  color: #b91c1c;
}
</style>