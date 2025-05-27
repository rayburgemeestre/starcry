import { defineStore } from 'pinia';
import { JsonWithObjectsParser } from 'src/core/json_parser';

export const useProjectStore = defineStore('script', {
  state: () => ({
    parser: null as JsonWithObjectsParser | null,
  }),

  getters: {},

  actions: {},
});
