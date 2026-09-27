import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'
import LoginView from '../views/LoginView.vue'
import EventDetailsView from '../views/EventDetailView.vue'
import CreateEventView from '../views/CreateEventView.vue'
import EditEventView from '../views/EditEventView.vue'
import StatisticsView from '../views/StatisticsView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView
    },
    {
      path: '/login',
      name: 'login',
      component: LoginView
    },
    {
      path: '/statistics',
      name: 'statistics',
      component: StatisticsView
    },
    {
      path: '/events/:id/edit',
      name: 'edit-event',
      component: EditEventView
    },
    {
      path: '/events/:id',
      name: 'event-details',
      component: EventDetailsView
    },
    {
      path: '/events/create',
      name: 'create-event',
      component: CreateEventView
    },
  ],
})

export default router
