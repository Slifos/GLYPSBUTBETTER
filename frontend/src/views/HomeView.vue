<script setup lang="ts">
import { eventService} from '../services/eventService'
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import {registrationService} from '../services/registrationService'
import { useUserStore } from '../stores/userStore'
import {type EventResponse} from '../types/event'

const router = useRouter()
const userStore = useUserStore()

const events = ref<EventResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)

const registeringEventId = ref<number | null>(null)
const registeredEventIds = ref<number[]>([])

onMounted(async () => {
    await loadEvents()
})

const loadEvents = async () => {
    loading.value = true
    error.value = null

    try {
        events.value = await eventService.getEvents()

        if(userStore.user){
            await loadUserRegistrations()
        }
    } catch (err) {
        console.error('Erreur lors de la récupération des événements:', err)
        error.value = 'Impossible de récupérer les événements. Veuillez réessayer plus tard.'
    } finally {
        loading.value = false
    }
}

const loadUserRegistrations = async () =>{
    if(!userStore.user) return

    const registrations: number[] = []

    for(const event of events.value){
        try{
            const eventRegistrations = await registrationService.getRegistrations(event.id)

            const isRegistered = eventRegistrations.some((registration) => registration.userId === userStore.user?.id)
            
            if(isRegistered){
                registrations.push(event.id)
            }
        }
        catch(err){
            console.error(`Erreur lors de la récupération des inscriptions pour l'événement ${event.id}:`, err)
        }
    }

    registeredEventIds.value = registrations
}

const isRegistered = (eventId: number): boolean => {
    return registeredEventIds.value.includes(eventId)
}

const joinEvent = async (event: EventResponse) =>{
    if(!userStore.user){
        router.push('/login')
        return
    }

    if (event.remainingPlaces <= 0) {
        alert("Désolé, cet événement est complet.")
        return
    }

    registeringEventId.value = event.id
    error.value = null

    try{
        await registrationService.registerForEvent(event.id, userStore.user.id)
        registeredEventIds.value.push(event.id)
        await loadEvents()
    } catch (err) {
        console.error(`Erreur lors de l'inscription à l'événement ${event.id}:`, err)
        error.value = 'Impossible de s\'inscrire à cet événement. Veuillez réessayer plus tard.'
    } finally {
        registeringEventId.value = null
    }
}

const leaveEvent = async (event: EventResponse) =>{
    if(!userStore.user){
        return
    }

    registeringEventId.value = event.id
    error.value = null

    try{
        await registrationService.cancelRegistration(event.id, userStore.user.id)
        registeredEventIds.value = registeredEventIds.value.filter(id => id !== event.id)
        await loadEvents()
    } catch (err) {
        console.error(`Erreur lors de la désinscription à l'événement ${event.id}:`, err)
        error.value = 'Impossible de se désinscrire de cet événement. Veuillez réessayer plus tard.'
    } finally {
        registeringEventId.value = null
    }
}
</script>

<template>
    <h1>GLYPSBUTBETTER</h1>

    <div class="user-info" v-if="userStore.user">
        <p>Bienvenue, {{ userStore.user?.name }} !</p>
        <button @click="userStore.logout()">Se déconnecter</button>
    </div>
    <div v-else>
        <p>Vous n'êtes pas connecté.</p>
        <button @click="router.push('/login')">Se connecter</button>
    </div>

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
                
                <button v-if="!userStore.user" @click="router.push('/login')">Se connecter</button>
                <button v-else-if="isRegistered(event.id)" :disabled="registeringEventId === event.id" @click="leaveEvent(event)">{{ registeringEventId === event.id ? 'Désinscription...' : 'Se désinscrire' }}</button>
                <button v-else-if="event.remainingPlaces <= 0" disabled>Complet</button>
                <button v-else :disabled="registeringEventId === event.id" @click="joinEvent(event)">{{ registeringEventId === event.id ? 'Inscription...' : 'S\'inscrire' }}</button>
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