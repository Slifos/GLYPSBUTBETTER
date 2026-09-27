<script setup lang="ts">
import { useUserStore } from './stores/userStore'

const userStore = useUserStore()
</script>

<template>
  <div class="app-shell">
    <header class="site-header">
      <div class="site-header__inner container">
        <RouterLink class="brand" to="/" aria-label="GLYPSBUTBETTER, accueil">
          <span class="brand__mark">G<span>.</span></span>
          <span class="brand__name">GLYPS<span>BUT</span>BETTER</span>
        </RouterLink>
        <nav class="site-nav" aria-label="Navigation principale">
          <RouterLink class="nav-events" to="/">Événements</RouterLink>
          <RouterLink to="/statistics">Statistiques</RouterLink>
          <RouterLink v-if="userStore.user" class="nav-create" to="/events/create">Créer un événement</RouterLink>
          <RouterLink v-if="!userStore.user" class="button button--small" to="/login">Se connecter <span aria-hidden="true">↗</span></RouterLink>
          <button v-else class="button button--small button--outline" type="button" @click="userStore.logout()">Déconnexion</button>
        </nav>
      </div>
    </header>
    <RouterView />
    <footer class="site-footer">
      <div class="container site-footer__inner">
        <span class="brand__name">GLYPS<span>BUT</span>BETTER</span>
        <span>Événements étudiants</span>
      </div>
    </footer>
  </div>
</template>
