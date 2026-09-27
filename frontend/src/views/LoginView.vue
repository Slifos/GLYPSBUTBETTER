```vue
<script setup lang="ts">
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import {type UserResponse} from '../types/user'
import { userService } from '../services/userService'
import { useUserStore } from '../stores/userStore'

const userStore = useUserStore()
const router = useRouter()

const email = ref('')
const name = ref('')
const creatingAccount = ref(false)
const error = ref<string | null>(null)
const loading = ref(false)

const submit = async () => {
  error.value = null

  if (!email.value.trim()) {
    error.value = 'Veuillez saisir votre adresse email.'
    return
  }

  loading.value = true

  try {
    if (creatingAccount.value) {
      if (!name.value.trim()) {
        error.value = 'Veuillez saisir votre nom.'
        return
      }
      const created = await userService.createUser({ name: name.value.trim(), email: email.value.trim() })
      userStore.login(created)
      await router.push('/')
      return
    }
    const users = await userService.getUsers()

    const user = users.find(
      (user: UserResponse) =>
        user.email.toLowerCase() === email.value.trim().toLowerCase()
    )

    if (!user) {
      error.value = 'Aucun utilisateur trouvé avec cette adresse email.'
      return
    }

    userStore.login(user)
    router.push('/')
  } catch (err) {
    console.error('Erreur lors de la connexion :', err)
    error.value = 'Impossible de contacter le serveur.'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <main class="login-page">
    <div class="login-card">
      <h1>GLYPSBUTBETTER</h1>

      <h2>{{ creatingAccount ? 'Créer un compte' : 'Connexion' }}</h2>

      <form @submit.prevent="submit">
        <div v-if="creatingAccount" class="form-group">
          <label for="name">Nom</label>
          <input id="name" v-model="name" type="text" autocomplete="name" />
        </div>
        <div class="form-group">
          <label for="email">Email</label>

          <input
            id="email"
            v-model="email"
            type="email"
            placeholder="exemple@campus.fr"
            autocomplete="email"
          />
        </div>

        <p v-if="error" class="error">
          {{ error }}
        </p>

        <button
          type="submit"
          :disabled="loading"
        >
          {{ loading ? 'Veuillez patienter...' : creatingAccount ? 'Créer le compte' : 'Se connecter' }}
        </button>
      </form>
      <button type="button" @click="creatingAccount = !creatingAccount; error = null">
        {{ creatingAccount ? 'J’ai déjà un compte' : 'Créer un compte' }}
      </button>
    </div>
  </main>
</template>

<style scoped>
.login-page {
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 20px;
}

.login-card {
  width: 100%;
  max-width: 400px;
  padding: 30px;
  border: 1px solid #ddd;
  border-radius: 12px;
  background: white;
}

.login-card h1 {
  text-align: center;
  margin-bottom: 10px;
}

.login-card h2 {
  text-align: center;
  margin-bottom: 30px;
}

.form-group {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-bottom: 20px;
}

input {
  padding: 12px;
  border: 1px solid #ccc;
  border-radius: 6px;
  font-size: 16px;
}

button {
  width: 100%;
  padding: 12px;
  border: none;
  border-radius: 6px;
  font-size: 16px;
  cursor: pointer;
}

button:disabled {
  cursor: not-allowed;
  opacity: 0.6;
}

.error {
  color: #d32f2f;
  margin-bottom: 15px;
}
</style>
```
