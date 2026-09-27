import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'
import LoginView from '../views/LoginView.vue'
import EventDetailsView from '../views/EventDetailView.vue'
import CreateEventView from '../views/CreateEventView.vue'
import EditEventView from '../views/EditEventView.vue'
import MyEventView from '../views/MyEventsView.vue'
import StatisticsDashboardView from '../views/StatisticsDashboardView.vue'
import CreateUserView from '../views/CreateUserView.vue'

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
    {
      path: '/my-events',
      name: 'my-events',
      component: MyEventView
    },
    {
      path: '/statistics',
      name: 'statistics',
      component: StatisticsDashboardView
    },
    {
      path: '/register',
      name: 'register',
      component: CreateUserView
    }
  ],
})

export default router
