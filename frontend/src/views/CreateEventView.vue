<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { eventService } from '../services/eventService'
import type { EventCreateRequest } from '../types/event'
import { useUserStore } from '../stores/userStore'

const router = useRouter()
const userStore = useUserStore()
const title = ref('')
const eventType = ref('')
const description = ref('')
const location = ref('')
const startDate = ref('')
const maxParticipants = ref<number | null>(null)

const loading = ref(false)
const error = ref<string | null>(null)

const createEvent = async () => {
  error.value = null

  if (!title.value.trim()) {
    error.value = "Le titre est obligatoire."
    return
  }

  if (!eventType.value.trim()) {
    error.value = "Le type d'événement est obligatoire."
    return
  }

  if (!startDate.value) {
    error.value = "La date de l'événement est obligatoire."
    return
  }

  if (
    maxParticipants.value === null ||
    maxParticipants.value <= 0
  ) {
    error.value =
      "Le nombre maximum de participants doit être supérieur à 0."
    return
  }

  const event: EventCreateRequest = {
    title: title.value.trim(),
    eventType: eventType.value.trim(),
    description: description.value.trim(),
    location: location.value.trim(),
    startDate: startDate.value,
    maxParticipants: maxParticipants.value,
  }

  loading.value = true

  try {
    const createdEvent =await eventService.createEvent(event)

    router.push({
      name: 'event-details',
      params: {
        id: createdEvent.id,
      },
    })
  } catch (err) {
    console.error("Erreur lors de la création de l'événement :",err,)
    error.value ="Impossible de créer l'événement."
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <main class="page container">
    <RouterLink class="back-link" to="/">← Retour aux événements</RouterLink>
    <div class="page-heading"><span class="eyebrow">Nouveau</span><h1>Créer un événement<span style="color:var(--orange)">.</span></h1><p>Renseigne les informations de l’événement.</p></div>
    <p v-if="!userStore.user" class="notice">Connecte-toi pour créer un événement. <RouterLink class="text-link" to="/login">Se connecter →</RouterLink></p>
    <form v-else class="event-form form-card surface" @submit.prevent="createEvent">
      <div class="field field--wide"><label for="title">Nom de l'événement</label><input id="title" v-model="title" type="text" placeholder="Ex. Soirée jeux de société" required /></div>
      <div class="field"><label for="eventType">Catégorie</label><input id="eventType" v-model="eventType" type="text" placeholder="Ex. Loisirs" required /></div>
      <div class="field"><label for="location">Lieu</label><input id="location" v-model="location" type="text" placeholder="Ex. Maison des étudiants" /></div>
      <div class="field field--wide"><label for="description">Description</label><textarea id="description" v-model="description" placeholder="Qu'est-ce qui attend les participants ?" rows="5"></textarea></div>
      <div class="field"><label for="startDate">Date et heure</label><input id="startDate" v-model="startDate" type="datetime-local" required /></div>
      <div class="field"><label for="maxParticipants">Nombre de places</label><input id="maxParticipants" v-model.number="maxParticipants" type="number" min="1" placeholder="Ex. 20" required /></div>
      <div class="form-actions"><p v-if="error" class="notice notice--error" role="alert">{{ error }}</p><button class="button" type="submit" :disabled="loading">{{ loading ? 'Création...' : "Créer l'événement" }} <span aria-hidden="true">→</span></button><RouterLink class="text-link" to="/">Annuler</RouterLink></div>
    </form>
  </main>
</template>
