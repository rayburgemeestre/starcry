import { defineStore } from 'pinia';

export type ObjectType = {
  unique_id: number;
  level: number;
  label: string;
  id: string;
  x: number;
  y: number;
  '#': number;
};

export const useObjectsStore = defineStore('objects', {
  state: () => ({
    objects: [] as ObjectType[],
    selected: [] as number[],
    lookup: {} as Record<number, number>,
  }),
  getters: {},
  actions: {
    addSelected(value: number) {
      if (!this.selected.includes(value)) {
        this.selected.push(value);
      }
    },
    removeSelected(value: number) {
      const index = this.selected.indexOf(value);
      if (index > -1) {
        this.selected.splice(index, 1);
      }
    },
    isSelected(value: number) {
      return this.selected.includes(value);
    },
    updateLookupTable() {
      const parentStack = []; // stack to keep track of parents at each level
      this.lookup = {};
      try {
        for (const obj of this.objects) {
          if (obj.level > 0) {
            // only non-root nodes have a parent
            this.lookup[obj.unique_id] = parentStack[obj.level - 1];
          }
          // update the current object as the potential parent for the next level
          parentStack[obj.level] = obj.unique_id;
        }
      } catch (e) {
        console.log('Error updating lookup table');
        console.log(e);
      }
    },
    parentLookup(unique_id: number) {
      return this.lookup[unique_id];
    },
  },
});
