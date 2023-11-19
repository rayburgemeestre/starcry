import { defineStore } from 'pinia';

export const useGlobalStore = defineStore('global', {
  state: () => ({
    connected: new Set<string>(),
    disconnected: new Set<string>(),
  }),
});
