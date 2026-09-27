<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'

import { userService } from '@/services/userService'

const router = useRouter()
const name = ref('')
const email = ref('')
const loading = ref(false)
const error = ref('')
const success = ref('')

const createUser = async () => {
  error.value = ''
  success.value = ''

  if (!name.value.trim()) {
    error.value = 'Le nom est obligatoire.'
    return
  }

  if (!email.value.trim()) {
    error.value = "L'adresse email est obligatoire."
    return
  }

  try {
    loading.value = true

    const user = await userService.createUser({name: name.value.trim(),email: email.value.trim()})

    success.value = `Utilisateur ${user.name} créé avec succès.`

    name.value = ''
    email.value = ''

    setTimeout(() => {
      router.push('/login')
    }, 1000)
  } catch (err) {
    console.error(err)
    error.value ="Impossible de créer l'utilisateur. Vérifie que l'adresse email n'est pas déjà utilisée."
  } finally {
    loading.value = false
  }
}
</script>

<template>
    <h1>Créer un compte</h1>

    <form @submit.prevent="createUser">

      <div>
        <label for="name">Nom</label>

        <input id="name" v-model="name" type="text" placeholder="Votre nom":disabled="loading"/>
      </div>

      <div>
        <label for="email">Email</label>
        <input id="email" v-model="email" type="email" placeholder="exemple@campus.fr" :disabled="loading"/>
      </div>

      <button type="submit" :disabled="loading">{{ loading ? 'Création...' : 'Créer mon compte' }}</button>
    </form>

    <p v-if="error">{{ error }}</p>
    <p v-if="success">{{ success }}</p>

    <button type="button" @click="router.push('/login')" :disabled="loading">Retour à la connexion</button>
</template>