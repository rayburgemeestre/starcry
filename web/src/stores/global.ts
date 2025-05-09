import { defineStore } from 'pinia';

export const useGlobalStore = defineStore('global', {
  state: () => ({
    show_connection_status: true,
    connected: new Set<string>(),
    disconnected: new Set<string>(),
  }),
});
