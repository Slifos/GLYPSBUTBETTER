import { ref } from 'vue'
import { defineStore } from 'pinia'
import { type UserResponse } from '../types/user'

export const useUserStore = defineStore('user', () => {
  const user = ref<UserResponse | null>(null)

  const isAuthenticated = ref(false)

  const login = (loggedUser: UserResponse) => {
    user.value = loggedUser
    isAuthenticated.value = true
  }

  const logout = () => {
    user.value = null
    isAuthenticated.value = false
  }

  return {
    user,
    isAuthenticated,
    login,
    logout
  }
})