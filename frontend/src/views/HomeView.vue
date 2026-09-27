<script setup lang="ts">
import { eventService} from '../services/eventService'
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import {registrationService} from '../services/registrationService'
import { useUserStore } from '../stores/userStore'
import {type EventResponse} from '../types/event'
import { notificationService, type Notification } from '../services/notificationService'

const router = useRouter()
const userStore = useUserStore()

const events = ref<EventResponse[]>([])
const loading = ref(true)
const error = ref<string | null>(null)

const registeringEventId = ref<number | null>(null)
const registeredEventIds = ref<number[]>([])
const notifications = ref<Notification[]>([])
const notificationError = ref<string | null>(null)

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
            await loadNotifications()
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

            const isRegistered = eventRegistrations.some((registration) =>
                registration.userId === userStore.user?.id && registration.status !== 'CANCELLED'
            )
            
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

const loadNotifications = async () => {
    if (!userStore.user) return
    try {
        notifications.value = await notificationService.forUser(userStore.user.id)
        notificationError.value = null
    } catch (err) {
        console.error('Erreur lors de la récupération des notifications:', err)
        notificationError.value = 'Impossible de charger les notifications.'
    }
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

const formatDate = (value: string) => new Intl.DateTimeFormat('fr-FR', {
    day: 'numeric', month: 'long', hour: '2-digit', minute: '2-digit'
}).format(new Date(value))
</script>

<template>
  <main>
    <section class="hero">
      <div class="container hero__inner">
        <div class="hero__content">
          <span class="eyebrow hero__eyebrow">Événements étudiants</span>
          <h1>Les événements<br /><em>du campus.</em></h1>
          <p>Consulte les événements à venir, inscris-toi et crée tes propres activités.</p>
          <div class="hero__actions">
            <a class="button button--light" href="#evenements">Explorer les événements <span aria-hidden="true">↗</span></a>
            <RouterLink v-if="userStore.user" class="hero__secondary" to="/events/create">Créer un événement <span aria-hidden="true">→</span></RouterLink>
            <RouterLink v-else class="hero__secondary" to="/login">Créer un compte <span aria-hidden="true">→</span></RouterLink>
          </div>
        </div>
        <div class="hero__art" aria-hidden="true">
          <div class="hero__circle hero__circle--back"></div>
          <div class="hero__circle hero__circle--front"><span>SPORT<br />ÉTUDES<br /><i>LOISIRS</i></span></div>
          <span class="hero__spark hero__spark--one">✳</span><span class="hero__spark hero__spark--two">✦</span>
        </div>
      </div>
    </section>

    <div class="container home-content" id="evenements">
      <div v-if="userStore.user" class="welcome-strip">
        <div><span class="eyebrow">Ton espace</span><p>Connecté en tant que <strong>{{ userStore.user.name }}</strong> 👋</p></div>
        <RouterLink class="text-link" to="/events/create">Créer un événement <span aria-hidden="true">→</span></RouterLink>
      </div>

      <section v-if="userStore.user" class="notifications surface" aria-labelledby="notifications-title">
        <div class="notifications__heading"><div><span class="eyebrow">Compte utilisateur</span><h2 id="notifications-title">Notifications</h2></div><button class="button button--small button--outline" type="button" @click="loadNotifications">Actualiser</button></div>
        <p v-if="notificationError" class="notice notice--error">{{ notificationError }}</p>
        <p v-else-if="notifications.length === 0" class="muted">Tu n'as aucune notification pour le moment.</p>
        <ul v-else class="notifications__list"><li v-for="notification in notifications" :key="notification.id">{{ notification.message }}</li></ul>
      </section>

      <section class="events-section" aria-labelledby="events-title">
        <div class="section-heading"><div><span class="eyebrow">Catalogue</span><h2 id="events-title">Événements à venir<span class="heading-dot">.</span></h2><p>Parcours les événements disponibles et consulte les places restantes.</p></div><span v-if="!loading && !error" class="event-count">{{ events.length }} événements</span></div>
        <p v-if="loading" class="notice">Chargement des événements...</p>
        <p v-else-if="error" class="notice notice--error">{{ error }}</p>
        <div v-else-if="events.length === 0" class="empty-state surface"><span class="empty-state__icon">✳</span><h3>Aucun événement disponible.</h3><p>Crée un événement pour qu’il apparaisse ici.</p><RouterLink class="button" to="/events/create">Créer un événement</RouterLink></div>
        <div v-else class="events-grid">
          <article v-for="(event, index) in events" :key="event.id" class="event-card surface">
            <div class="event-card__art" :class="`event-card__art--${index % 4}`"><span class="event-card__symbol">{{ ['✳','✦','◉','✺'][index % 4] }}</span><span class="event-card__category">{{ event.eventType }}</span></div>
            <div class="event-card__body">
              <p class="event-card__date">{{ formatDate(event.startDate) }}</p>
              <h3><RouterLink :to="{name: 'event-details', params: {id: event.id}}">{{ event.title }}</RouterLink></h3>
              <p class="event-card__description">{{ event.description || 'Aucune description fournie.' }}</p>
              <div class="event-card__meta"><span>⌁ {{ event.location || 'Lieu à préciser' }}</span><span :class="{'is-full': event.remainingPlaces === 0}">{{ event.remainingPlaces === 0 ? 'Complet' : `${event.remainingPlaces} places` }}</span></div>
              <div class="event-card__footer"><RouterLink class="text-link" :to="{name: 'event-details', params: {id: event.id}}">Voir l'événement <span aria-hidden="true">→</span></RouterLink><button v-if="userStore.user && isRegistered(event.id)" type="button" class="event-card__quick" :disabled="registeringEventId === event.id" @click="leaveEvent(event)">Se désinscrire</button><button v-else-if="userStore.user && event.remainingPlaces > 0" type="button" class="event-card__quick" :disabled="registeringEventId === event.id" @click="joinEvent(event)">Rejoindre</button></div>
            </div>
          </article>
        </div>
      </section>
    </div>
  </main>
</template>

<style scoped>
.hero{overflow:hidden;background:#174f42;color:#fff}.hero__inner{min-height:480px;display:grid;grid-template-columns:minmax(0,1.2fr) minmax(270px,.8fr);align-items:center;gap:28px}.hero__content{position:relative;z-index:1;padding:65px 0}.hero__eyebrow{color:#f5b694}.hero h1{max-width:760px;margin:20px 0;font-size:clamp(43px,6.5vw,78px);line-height:1.04;letter-spacing:-.065em}.hero h1 em{color:#f6a77d;font-style:normal}.hero p{max-width:540px;color:#d4e6dc;font-size:17px;line-height:1.7}.hero__actions{display:flex;align-items:center;gap:25px;flex-wrap:wrap;margin-top:32px}.hero__secondary{color:#fff;font-size:14px;font-weight:700}.hero__secondary:hover{text-decoration:underline}.hero__art{position:relative;min-height:390px}.hero__circle{position:absolute;border-radius:50%}.hero__circle--back{width:330px;height:330px;top:25px;right:-45px;border:1px solid #ffffff38;background:#2d6658}.hero__circle--front{display:grid;place-items:center;width:295px;height:295px;top:62px;right:62px;transform:rotate(-12deg);background:#f5ad82;color:#174f42;box-shadow:16px 21px 0 #0d3e35}.hero__circle--front span{font-family:'Space Grotesk';font-size:39px;font-weight:700;line-height:1.04;letter-spacing:-.06em}.hero__circle--front i{font-style:normal;color:#fff}.hero__spark{position:absolute;color:#f6bd98;font-size:55px}.hero__spark--one{top:10px;left:8px}.hero__spark--two{right:0;bottom:0;font-size:92px}.home-content{padding-top:60px;padding-bottom:90px}.welcome-strip{display:flex;align-items:center;justify-content:space-between;gap:24px;margin-bottom:38px;padding:20px 26px;border-left:4px solid var(--orange);background:#fff4e9}.welcome-strip p{margin:7px 0 0;font-family:'Space Grotesk';font-size:20px}.notifications{padding:26px 30px;margin-bottom:46px}.notifications__heading{display:flex;align-items:center;justify-content:space-between;gap:15px}.notifications h2{margin:5px 0 12px;font-size:23px}.notifications p{margin-bottom:0}.notifications__list{margin:0;padding-left:20px;line-height:1.8;color:var(--muted)}.section-heading{display:flex;align-items:end;justify-content:space-between;gap:20px;margin-bottom:28px}.section-heading h2{margin:8px 0;font-size:clamp(31px,4.5vw,48px)}.section-heading p{margin:0;color:var(--muted)}.heading-dot{color:var(--orange)}.event-count{flex:none;padding:9px 13px;border-radius:30px;background:var(--green-light);color:var(--green);font-size:12px;font-weight:800}.events-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:22px}.event-card{overflow:hidden;display:flex;flex-direction:column;transition:transform .2s,box-shadow .2s}.event-card:hover{transform:translateY(-4px);box-shadow:0 18px 35px #18332f16}.event-card__art{position:relative;display:flex;align-items:center;justify-content:center;height:145px;background:#d9ebe2;color:#2a7665}.event-card__art--1{background:#f6dfcc;color:#c26543}.event-card__art--2{background:#e4e2ef;color:#7770aa}.event-card__art--3{background:#e9ebd1;color:#7e8b48}.event-card__symbol{font-family:'Space Grotesk';font-size:100px;line-height:1;opacity:.68}.event-card__category{position:absolute;left:16px;bottom:14px;max-width:calc(100% - 32px);padding:7px 11px;border-radius:30px;background:#fffefa;color:var(--ink);font-size:11px;font-weight:800;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.event-card__body{display:flex;flex:1;flex-direction:column;padding:23px}.event-card__date{margin:0 0 10px;color:#ba6243;font-size:12px;font-weight:800;text-transform:uppercase;letter-spacing:.04em}.event-card h3{margin:0 0 9px;font-size:23px;line-height:1.2}.event-card h3 a:hover{color:var(--orange)}.event-card__description{display:-webkit-box;min-height:48px;margin:0 0 18px;overflow:hidden;color:var(--muted);font-size:13px;line-height:1.55;-webkit-line-clamp:2;-webkit-box-orient:vertical}.event-card__meta{display:flex;justify-content:space-between;gap:10px;padding:14px 0;border-top:1px solid var(--line);color:var(--muted);font-size:12px}.event-card__meta span:last-child{color:var(--green);font-weight:800;white-space:nowrap}.event-card__meta .is-full{color:#af4c34!important}.event-card__footer{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-top:auto;padding-top:13px}.event-card__quick{padding:7px 9px;border:0;background:transparent;color:var(--orange);font-size:12px;font-weight:800}.event-card__quick:hover{text-decoration:underline}.empty-state{text-align:center;padding:60px 25px}.empty-state__icon{font-size:60px;color:var(--orange)}.empty-state h3{margin:10px 0;font-size:25px}.empty-state p{color:var(--muted)}
@media(max-width:950px){.events-grid{grid-template-columns:repeat(2,minmax(0,1fr))}.hero__circle--back{right:-145px}.hero__circle--front{right:-25px}}@media(max-width:700px){.hero__inner{display:block;min-height:0}.hero__content{padding:60px 0 70px}.hero__art{display:none}.hero p{font-size:15px}.home-content{padding-top:42px}.events-grid{grid-template-columns:1fr}.section-heading{align-items:start}.event-count{display:none}.welcome-strip{align-items:start;flex-direction:column}.notifications{padding:22px}}
</style>
