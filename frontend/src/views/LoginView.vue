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
    <div class="container login-layout">
      <section class="login-intro"><span class="eyebrow">Compte utilisateur</span><h1>Connexion aux <em>événements.</em></h1><p>Connecte-toi pour t’inscrire aux événements et en créer.</p><div class="login-intro__deco" aria-hidden="true">✳ <span>✦</span></div></section>
      <section class="login-card surface" aria-labelledby="login-title"><span class="eyebrow">Accès</span><h2 id="login-title">{{ creatingAccount ? 'Créer un compte' : 'Connexion' }}</h2><p class="muted">{{ creatingAccount ? 'Renseigne ton nom et ton adresse email.' : 'Saisis l’adresse email de ton compte.' }}</p>
        <form class="login-form" @submit.prevent="submit">
          <div v-if="creatingAccount" class="field"><label for="name">Ton nom</label><input id="name" v-model="name" type="text" autocomplete="name" placeholder="Ex. Camille Martin" required /></div>
          <div class="field"><label for="email">Adresse email</label><input id="email" v-model="email" type="email" autocomplete="email" placeholder="exemple@campus.fr" required /></div>
          <p v-if="error" class="notice notice--error" role="alert">{{ error }}</p>
          <button class="button" type="submit" :disabled="loading">{{ loading ? 'Veuillez patienter...' : creatingAccount ? 'Créer mon compte' : 'Se connecter' }} <span aria-hidden="true">→</span></button>
        </form>
        <p class="login-switch">{{ creatingAccount ? 'Déjà un compte ?' : 'Pas encore de compte ?' }} <button type="button" @click="creatingAccount = !creatingAccount; error = null">{{ creatingAccount ? 'Se connecter' : 'Créer un compte' }}</button></p>
      </section>
    </div>
  </main>
</template>

<style scoped>
.login-page{display:flex;align-items:center;flex:1;padding:65px 0 90px;background:radial-gradient(circle at 14% 20%,#e0ece3 0,transparent 33%),#f7f5ef}.login-layout{display:grid;grid-template-columns:1fr 460px;gap:8%;align-items:center}.login-intro h1{max-width:600px;margin:19px 0;font-size:clamp(43px,5.5vw,70px);line-height:1.08}.login-intro em{color:var(--orange);font-style:normal}.login-intro p{max-width:420px;color:var(--muted);font-size:16px;line-height:1.7}.login-intro__deco{margin-top:28px;color:var(--orange);font-size:80px;line-height:1}.login-intro__deco span{color:var(--green);font-size:44px;vertical-align:top}.login-card{padding:clamp(26px,5vw,48px)}.login-card h2{margin:10px 0 7px;font-size:34px}.login-card>.muted{font-size:14px;line-height:1.55}.login-form{display:flex;flex-direction:column;gap:20px;margin-top:28px}.login-form .button{width:100%}.login-form .notice{margin:0}.login-switch{margin:24px 0 0;padding-top:20px;border-top:1px solid var(--line);color:var(--muted);font-size:13px;text-align:center}.login-switch button{padding:0;border:0;background:transparent;color:var(--green);font-weight:800}.login-switch button:hover{text-decoration:underline}@media(max-width:800px){.login-layout{grid-template-columns:1fr;gap:35px}.login-intro__deco{display:none}.login-intro h1{font-size:42px}.login-card{max-width:560px}}
</style>
