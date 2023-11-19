import { defineStore } from 'pinia';
import { ObjectType } from 'stores/objects';

export const useStatsStore = defineStore('stats', {
  state: () => ({
    statistics: [] as ObjectType[],
    metrics: [] as ObjectType[],
    status: '' as string,
    rows: [] as ObjectType[],
    rows_piper: [] as ObjectType[],
  }),
});
