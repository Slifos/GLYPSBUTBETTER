<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { eventService } from '../services/eventService'
import type {EventResponse, EventUpdateRequest} from '../types/event'

const route = useRoute()
const router = useRouter()
const event = ref<EventResponse | null>(null)
const title = ref('')
const eventType = ref('')
const description = ref('')
const location = ref('')
const startDate = ref('')
const maxParticipants = ref<number | null>(null)
const loading = ref(true)
const saving = ref(false)
const error = ref<string | null>(null)
const eventId = Number(route.params.id)

const loadEvent = async () => {
  loading.value = true
  error.value = null

  try {
    event.value = await eventService.getEventById(eventId)

    title.value = event.value.title
    eventType.value = event.value.eventType
    description.value = event.value.description
    location.value = event.value.location
    startDate.value = event.value.startDate
    maxParticipants.value = event.value.maxParticipants
  } catch (err) {
    console.error("Erreur lors du chargement de l'événement :",err)

    error.value ="Impossible de charger cet événement."
  } finally {
    loading.value = false
  }
}

const updateEvent = async () => {
  error.value = null

  if (!title.value.trim()) {
    error.value = 'Le titre est obligatoire.'
    return
  }

  if (!eventType.value.trim()) {
    error.value ="Le type d'événement est obligatoire."
    return
  }

  if (!startDate.value) {
    error.value ="La date de l'événement est obligatoire."
    return
  }

  if (maxParticipants.value === null || maxParticipants.value <= 0) {
    error.value ="Le nombre maximum de participants doit être supérieur à 0."
    return
  }

  const updatedEvent: EventUpdateRequest = {
    title: title.value.trim(),
    eventType: eventType.value.trim(),
    description: description.value.trim(),
    location: location.value.trim(),
    startDate: startDate.value,
    maxParticipants: maxParticipants.value,
  }

  saving.value = true

  try {
    await eventService.updateEvent(eventId, updatedEvent)

    router.push({name: 'event-details',params: {id: eventId}})
  } catch (err) {
    console.error(
      "Erreur lors de la modification de l'événement :",err)

    error.value ="Impossible de modifier l'événement."
  } finally {
    saving.value = false
  }
}

onMounted(() => {
  loadEvent()
})
</script>

<template>
  <main class="page container">
    <RouterLink class="back-link" :to="{name:'event-details',params:{id:eventId}}">← Retour à l'événement</RouterLink>
    <div class="page-heading"><span class="eyebrow">Mise à jour</span><h1>Modifier l'événement<span style="color:var(--orange)">.</span></h1><p>Modifie les informations de l’événement.</p></div>
    <p v-if="loading" class="notice">Chargement...</p>
    <p v-else-if="error && !event" class="notice notice--error" role="alert">{{ error }}</p>
    <form v-else class="event-form form-card surface" @submit.prevent="updateEvent">
      <div class="field field--wide"><label for="title">Nom de l'événement</label><input id="title" v-model="title" type="text" required /></div>
      <div class="field"><label for="eventType">Catégorie</label><input id="eventType" v-model="eventType" type="text" required /></div>
      <div class="field"><label for="location">Lieu</label><input id="location" v-model="location" type="text" /></div>
      <div class="field field--wide"><label for="description">Description</label><textarea id="description" v-model="description" rows="5"></textarea></div>
      <div class="field"><label for="startDate">Date et heure</label><input id="startDate" v-model="startDate" type="datetime-local" required /></div>
      <div class="field"><label for="maxParticipants">Nombre de places</label><input id="maxParticipants" v-model.number="maxParticipants" type="number" min="1" required /></div>
      <div class="form-actions"><p v-if="error" class="notice notice--error" role="alert">{{ error }}</p><button class="button" type="submit" :disabled="saving">{{ saving ? 'Enregistrement...' : 'Enregistrer les modifications' }} <span aria-hidden="true">→</span></button><RouterLink class="text-link" :to="{name:'event-details',params:{id:eventId}}">Annuler</RouterLink></div>
    </form>
  </main>
</template>
