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
const error = ref<string | null>(null)
const loading = ref(false)

const login = async () => {
  error.value = null

  if (!email.value.trim()) {
    error.value = 'Veuillez saisir votre adresse email.'
    return
  }

  loading.value = true

  try {
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
    console.log('Utilisateur connecté :', userStore.user)

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

      <h2>Connexion</h2>

      <form @submit.prevent="login">
        <div class="form-group">
          <label for="email">Email</label>

          <input id="email" v-model="email" type="email" placeholder="exemple@campus.fr" autocomplete="email"/>
        </div>

        <p v-if="error" class="error">
          {{ error }}
        </p>

        <button type="submit" :disabled="loading">{{ loading ? 'Connexion...' : 'Se connecter' }}</button>
      </form>

      <p>Pas encore de compte ? <RouterLink to="/register">Créer un compte</RouterLink></p>
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
