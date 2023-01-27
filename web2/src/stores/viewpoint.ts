import { defineStore } from 'pinia';

export const useViewpointStore = defineStore('viewpoint', {
  state: () => ({
    scale: 1.
  }),
});
