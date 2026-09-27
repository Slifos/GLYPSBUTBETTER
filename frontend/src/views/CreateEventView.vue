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
  <main>
    <button @click="router.push('/')">Retour aux événements</button>

    <h1>Créer un événement</h1>

    <p v-if="!userStore.user">Vous devez être connecté pour créer un événement.</p>

    <form v-else @submit.prevent="createEvent">
      <div>
        <label for="title">Titre</label>

        <input id="title" v-model="title" type="text" placeholder="Ex : Soirée étudiante"/>
      </div>

      <div>
        <label for="eventType">Type d'événement</label>

        <input
          id="eventType"
          v-model="eventType"
          type="text"
          placeholder="Ex : SOCIAL"
        />
      </div>

      <div>
        <label for="description">Description</label>

        <textarea id="description" v-model="description" placeholder="Décris ton événement..." rows="5"></textarea>
      </div>

      <div>
        <label for="location">Lieu</label>

        <input
          id="location"
          v-model="location"
          type="text"
          placeholder="Ex : Campus"
        />
      </div>

      <div>
        <label for="startDate">Date et heure</label>

        <input id="startDate" v-model="startDate" type="datetime-local"/>
      </div>

      <div>
        <label for="maxParticipants">Nombre maximum de participants</label>

        <input
          id="maxParticipants"
          v-model.number="maxParticipants"
          type="number"
          min="1"
          placeholder="Ex : 50"
        />
      </div>

      <p v-if="error">{{ error }}</p>

      <button type="submit" :disabled="loading">{{loading ? "Création..." : "Créer l'événement"}}</button>
    </form>
  </main>
</template>

