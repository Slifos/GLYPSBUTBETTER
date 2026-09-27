<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { eventService } from '../services/eventService'
import { registrationService } from '../services/registrationService'
import { useUserStore } from '../stores/userStore'
import { type EventResponse } from '../types/event'

const router = useRouter()
const userStore = useUserStore()
const events = ref<EventResponse[]>([])
const loading = ref(true)
const error = ref('')

const loadMyEvents = async () => {
    if (!userStore.user) {
        router.push('/login')
        return
    }

    try {
        loading.value = true
        error.value = ''

        const allEvents = await eventService.getEvents()
        const myEvents: EventResponse[] = []

        for (const event of allEvents) {
            const registrations = await registrationService.getRegistrations(event.id)
            
            const isRegistered = registrations.some((registration) => registration.userId === userStore.user?.id && registration.status === 'ACTIVE')

            if (isRegistered) { myEvents.push(event)}
        }
        events.value = myEvents
    } catch (err) {
        console.error(err)
        error.value = 'Impossible de charger vos inscriptions.'
    } finally {
        loading.value = false
    }
}

const openEvent = (eventId: number) => {
  router.push({name: 'event-details',params: { id: eventId }})
}

onMounted(loadMyEvents)
</script>

<template>
    <h1>Mes Inscriptions</h1>
    <p v-if="loading">Chargement...</p>
    <p v-else-if="error">{{ error }}</p>
    <p v-else-if="events.length === 0">Vous n'êtes inscrit à aucun événement.</p>

    <section v-else>
        <article v-for="event in events" :key="event.id">
            <h3 @click="openEvent(event.id)" style="cursor: pointer">
                {{ event.title }}
            </h3>
            <p>Type: {{ event.eventType }}</p>
            <p>Lieu: {{ event.location }}</p>
            <p class="remaining-places" :class="{'full': event.remainingPlaces === 0, 'few-places': event.remainingPlaces > 0 && event.remainingPlaces <= 10}">
                {{ event.remainingPlaces }} places restantes
            </p>
            <p>{{ event.currentParticipants }} / {{ event.maxParticipants }} participants</p>
            <button @click="openEvent(event.id)">Voir les détails</button>
        </article>
    </section>
</template>