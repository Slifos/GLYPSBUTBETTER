<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { eventService } from '../services/eventService'
import { useUserStore } from '../stores/userStore'
import { registrationService } from '../services/registrationService'
import { type EventResponse } from '../types/event'
import { type RegistrationResponse } from '../types/registration'

const route = useRoute()
const router = useRouter()
const userStore = useUserStore()
const event = ref<EventResponse | null>(null)
const registrations = ref<RegistrationResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)
const processingRegistration = ref(false)
const eventId = Number(route.params.id)

onMounted(() => {
    loadEvent()
})

const loadEvent = async () => {
    loading.value = true
    error.value = null

    try {
        event.value = await eventService.getEventById(eventId)
        registrations.value = await registrationService.getRegistrations(eventId)

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

const formatDate = (value: string) => new Intl.DateTimeFormat('fr-FR', {
  weekday: 'long', day: 'numeric', month: 'long', year: 'numeric', hour: '2-digit', minute: '2-digit'
}).format(new Date(value))
</script>

<template>
  <main class="page container">
    <RouterLink class="back-link" to="/">← Retour aux événements</RouterLink>
    <p v-if="loading" class="notice">Chargement de l'événement...</p>
    <p v-else-if="error && !event" class="notice notice--error" role="alert">{{ error }}</p>
    <div v-else-if="event" class="detail-layout">
      <section class="detail-main">
        <div class="detail-banner"><span class="detail-banner__symbol" aria-hidden="true">✳</span><span class="detail-banner__tag">{{ event.eventType }}</span></div>
        <div class="detail-content"><span class="eyebrow">Événement</span><h1>{{ event.title }}<span style="color:var(--orange)">.</span></h1><p class="detail-lead">{{ event.description || 'Aucune description fournie.' }}</p><h2>Informations</h2><div class="detail-facts"><div><span class="detail-facts__icon" aria-hidden="true">◷</span><span><small>Date et heure</small><strong>{{ formatDate(event.startDate) }}</strong></span></div><div><span class="detail-facts__icon" aria-hidden="true">⌁</span><span><small>Lieu</small><strong>{{ event.location || 'À préciser' }}</strong></span></div></div></div>
      </section>
      <aside class="detail-sidebar"><div class="detail-join surface"><span class="eyebrow">Inscription</span><h2>{{ event.remainingPlaces === 0 ? 'Complet' : `${event.remainingPlaces} places restantes` }}</h2><p>{{ event.currentParticipants }} participant{{ event.currentParticipants > 1 ? 's' : '' }} sur {{ event.maxParticipants }} places</p><div class="capacity-track"><span :style="{width: `${(event.currentParticipants / event.maxParticipants) * 100}%`}"></span></div><p v-if="error" class="notice notice--error" role="alert">{{ error }}</p><button v-if="!userStore.user" class="button" type="button" @click="router.push('/login')">Se connecter pour participer</button><button v-else-if="isRegistered()" class="button button--outline" type="button" :disabled="processingRegistration" @click="leaveEvent()">{{ processingRegistration ? 'Désinscription...' : 'Se désinscrire' }}</button><button v-else-if="event.remainingPlaces <= 0" class="button" disabled>Événement complet</button><button v-else class="button" type="button" :disabled="processingRegistration" @click="joinEvent()">{{ processingRegistration ? 'Inscription...' : "Je m'inscris" }} <span aria-hidden="true">→</span></button></div><div class="detail-admin"><RouterLink class="text-link" :to="{name:'edit-event',params:{id:eventId}}">Modifier l'événement</RouterLink><button type="button" :disabled="processingRegistration" @click="deleteEvent">Supprimer</button></div></aside>
    </div>
  </main>
</template>

<style scoped>
.detail-layout{display:grid;grid-template-columns:minmax(0,1fr) 320px;gap:30px;align-items:start}.detail-main{overflow:hidden;border:1px solid var(--line);border-radius:21px;background:var(--paper)}.detail-banner{position:relative;display:grid;place-items:center;height:245px;background:#d9ebe2;color:#2a7665}.detail-banner__symbol{font-size:155px;line-height:1;opacity:.65}.detail-banner__tag{position:absolute;left:28px;bottom:24px;padding:8px 14px;border-radius:20px;background:#fff;color:var(--ink);font-size:12px;font-weight:800}.detail-content{padding:clamp(26px,4vw,45px)}.detail-content h1{margin:10px 0 20px;font-size:clamp(38px,5vw,60px);line-height:1.06}.detail-lead{color:var(--muted);font-size:17px;line-height:1.75;white-space:pre-line}.detail-content h2{margin:35px 0 18px;font-size:23px}.detail-facts{display:grid;grid-template-columns:1fr 1fr;gap:14px}.detail-facts>div{display:flex;gap:13px;align-items:center;padding:18px;border:1px solid var(--line);border-radius:13px;background:#f9faf5}.detail-facts__icon{display:grid;place-items:center;flex:none;width:40px;height:40px;border-radius:10px;background:var(--green-light);color:var(--green);font-size:24px}.detail-facts small{display:block;margin-bottom:4px;color:var(--muted);font-size:12px}.detail-facts strong{font-size:13px}.detail-join{padding:27px}.detail-join h2{margin:10px 0 3px;font-size:29px}.detail-join>p:not(.notice){color:var(--muted);font-size:13px}.detail-join .button{width:100%;margin-top:23px}.detail-join .notice{margin-top:18px}.capacity-track{height:8px;margin-top:19px;overflow:hidden;border-radius:8px;background:#e1e8dd}.capacity-track span{display:block;height:100%;border-radius:8px;background:var(--orange)}.detail-admin{display:flex;align-items:center;justify-content:space-between;gap:14px;margin-top:20px;padding:0 5px}.detail-admin button{border:0;background:none;color:#a84731;font-size:13px;font-weight:700}.detail-admin button:hover{text-decoration:underline}@media(max-width:800px){.detail-layout{grid-template-columns:1fr}.detail-sidebar{grid-row:2}.detail-facts{grid-template-columns:1fr}}
</style>
