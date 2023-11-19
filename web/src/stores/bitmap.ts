import { defineStore } from 'pinia';
import { ObjectType } from 'stores/objects';

export const useBitmapStore = defineStore('bitmap', {
  state: () => ({
    loading: false as boolean,
    outbox: [] as ObjectType[],
  }),
});
