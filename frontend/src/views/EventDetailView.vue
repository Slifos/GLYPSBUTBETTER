<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { eventService } from '../services/eventService'
import { useUserStore } from '../stores/userStore'
import { registrationService } from '../services/registrationService'
import { type EventResponse } from '../types/event'
import { type RegistrationResponse } from '../types/registration'
import {statisticsService} from '../services/statisticsService'
import type {EventStatistics} from '../types/statistics'
import {userService} from '../services/userService'
import type {UserResponse} from '../types/user'

const route = useRoute()
const router = useRouter()
const userStore = useUserStore()
const event = ref<EventResponse | null>(null)
const registrations = ref<RegistrationResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)
const processingRegistration = ref(false)
const eventId = Number(route.params.id)
const statistics = ref<EventStatistics | null>(null)
const users = ref<Record<number, UserResponse>>({})
const processingAttendance = ref<number | null>(null)

onMounted(() => {
    loadEvent()
})

const loadEvent = async () => {
    loading.value = true
    error.value = null

    try {
        event.value = await eventService.getEventById(eventId)
        registrations.value = await registrationService.getRegistrations(eventId)
        await loadParticipantUsers()
        statistics.value = await statisticsService.getEventStatistics(eventId)

    } catch (err) {
        console.error('Erreur lors du chargement de l\'événement:', err)
        error.value = 'Impossible de charger l\'événement. Veuillez réessayer plus tard.'
    } finally {
        loading.value = false
    }
}

const isRegistered = () => {
    if (!userStore.user) {
        return false
    }
    return registrations.value.some(
        (registration) => registration.userId === userStore.user?.id && registration.status === "ACTIVE"
    )
}

const joinEvent = async () => {
    if (!userStore.user) {
        router.push('/login')
        return
    }
    if (!event.value || event.value.remainingPlaces <= 0) {
        return

    }
    processingRegistration.value = true
    error.value = null
    try {
        await registrationService.registerForEvent( eventId, userStore.user.id, )
        await loadEvent()
    } catch (err) {
            console.error( "Erreur lors de l'inscription :", err, )
            error.value = "Impossible de rejoindre cet événement."
    } finally {
        processingRegistration.value = false
    }
}

const leaveEvent = async () => {
    if (!userStore.user) {
        return
    }
    processingRegistration.value = true
    error.value = null
    try {
        await registrationService.cancelRegistration( eventId, userStore.user.id, )
        await loadEvent()
    } catch (err) {
        console.error( "Erreur lors de la désinscription :", err, )
        error.value = "Impossible de se désinscrire de cet événement."
    } finally {
        processingRegistration.value = false
    }
}

const deleteEvent = async () => {
  if (!event.value) {return}

  const confirmed = window.confirm('Voulez-vous vraiment supprimer cet événement ?')

  if (!confirmed) {return}

  processingRegistration.value = true
  error.value = null

  try {
    await eventService.deleteEvent(event.value.id)
    router.push('/')
  } catch (err) {
    console.error("Erreur lors de la suppression :",err)
    error.value ="Impossible de supprimer l'événement."
  } finally {
    processingRegistration.value = false
  }
}

const loadParticipantUsers = async () => {
    try {
        const userIds = [
            ...new Set(registrations.value.map((registration) => registration.userId)),
        ]

        const loadedUsers = await Promise.all(userIds.map((userId) =>userService.getUserById(userId),))
        users.value = {}
        loadedUsers.forEach((user) => {users.value[user.id] = user})
    } catch (err) {
        console.error('Erreur lors du chargement des participants :',err)
    }
}

const getParticipantName = (userId: number) => {
    const user = users.value[userId]

    if (!user) {return `Utilisateur ${userId}`}
    return user.name
}

const markAttended = async (userId: number) => {
    processingAttendance.value = userId
    error.value = null

    try {
        await registrationService.markAttended(eventId,userId)

        await loadEvent()
        await loadParticipantUsers()
    } catch (err) {
        console.error('Erreur lors du marquage de la présence :',err)

        error.value ='Impossible de marquer le participant comme présent.'
    } finally {
        processingAttendance.value = null
    }
}
</script>

<template>
    <button @click="router.push('/')">Retour aux événements</button>
    <p v-if="loading">Chargement de l'événement...</p>
    <p v-else-if="error">{{ error }}</p>
    <section v-else-if="event">
        <h1>{{ event.title }}</h1>
        <p><strong>Type:</strong> {{ event.eventType }}</p>
        <p><strong>Description:</strong> {{ event.description }}</p>
        <p><strong>Lieu:</strong> {{ event.location }}</p>
        <p><strong>Date:</strong> {{ event.startDate }}</p>
        <p><strong>Places restantes:</strong> {{ event.remainingPlaces }}</p>
        <p><strong>Participants:</strong>{{ event.currentParticipants }}/ {{ event.maxParticipants }}</p>

        <br />
        <button v-if="!userStore.user" @click="router.push('/login')">Se connecter</button>
        <button v-else-if="isRegistered()" :disabled="processingRegistration" @click="leaveEvent()">{{ processingRegistration ? 'Désinscription...' : 'Se désinscrire' }}</button>
        <button v-else-if="event.remainingPlaces <= 0" disabled>Complet</button>
        <button v-else :disabled="processingRegistration" @click="joinEvent()">{{ processingRegistration ? 'Inscription...' : 'S\'inscrire' }}</button>

        <button @click="router.push({name: 'edit-event', params: { id: eventId }})">Modifier</button>
        <button :disabled="processingRegistration" @click="deleteEvent">Supprimer</button>
    </section>
    <section>
        <hr />

        <h2>Participants</h2>

        <p v-if="registrations.length === 0">Aucun participant inscrit.</p>

        <div v-for="registration in registrations" :key="registration.id">
            <p>
                <strong>{{ getParticipantName(registration.userId) }}</strong>

                <span v-if="registration.status === 'ACTIVE'">— Inscrit</span>

                <span v-else-if="registration.status === 'ATTENDED'">— Présent</span>

                <span v-else>— {{ registration.status }}</span>
            </p>

            <button
                v-if="registration.status === 'ACTIVE'"
                :disabled="
                    processingAttendance === registration.userId
                "
                @click="markAttended(registration.userId)">
                {{processingAttendance === registration.userId ? 'Enregistrement...' : 'Marquer présent'}}
            </button>
        </div>
    </section>
    <section v-if="statistics">
        <hr />
        <h2>Statistiques</h2>
        <p><strong>Places restantes :</strong>{{ statistics.remainingPlaces }}</p>
        <p><strong>Taux de remplissage :</strong>{{ statistics.occupancyRate }} %</p>
        <p><strong>Participants présents :</strong>{{ statistics.attendedParticipants }}</p>
        <p><strong>Taux de présence :</strong>{{ statistics.attendanceRate }} %</p>
        <p><strong>Taux d'annulation :</strong>{{ statistics.cancellationRate }} %</p>
        <p><strong>Inscriptions par jour :</strong>{{ statistics.registrationsPerDay }}</p>
        <p v-if="statistics.estimatedFullAt"><strong>Remplissage estimé :</strong>{{ statistics.estimatedFullAt }}</p>
        <p v-if="statistics.full">Cet événement est complet.</p>
    </section>

</template>