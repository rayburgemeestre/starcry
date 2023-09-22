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
    implied_selected: [] as number[],
    lookup: {} as Record<number, number>,
  }),
  getters: {},
  actions: {
    addSelected(value: number) {
      if (!this.selected.includes(value)) {
        this.selected.push(value);
      }
    },
    addImpliedSelected(value: number) {
     if (!this.implied_selected.includes(value)) {
        this.implied_selected.push(value);
      }
    },
    removeSelected(value: number) {
      const index = this.selected.indexOf(value);
      if (index > -1) {
        this.selected.splice(index, 1);
      }
    },
    removeImpliedSelected(value: number) {
      const index = this.implied_selected.indexOf(value);
      if (index > -1) {
        this.implied_selected.splice(index, 1);
      }
    },
    isSelected(value: number) {
      if (this.selected.includes(value)) {
        return 1;
      } else if (this.implied_selected.includes(value)) {
        return 2;
      }
      return 0;
    },
    isSelectedArray(value: number[]) {
      for (const v of value) {
        if (this.isSelected(v) <= 0) {
          return false;
        }
      }
      return true;
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
    parentsLookup(unique_id: number) {
      let currentID = unique_id;
      const ids = [];
      while (currentID !== undefined && this.lookup[currentID] !== undefined) {
        currentID = this.lookup[currentID];
        ids.push(currentID);
      }
      return ids;
    },
    onUserSelected(unique_id: number) {
      let currentID = unique_id;
      const ids = [];

      this.selected.push(unique_id);

      while (currentID !== undefined && this.lookup[currentID] !== undefined) {
        ids.push(currentID);
        currentID = this.lookup[currentID];
      }

      if (currentID !== undefined) {
        ids.push(currentID); // add the last id which doesn't have a parent in lookup
      }

      this.implied_selected = [...new Set([...this.selected, ...ids])];
    },
    reset() {
      this.selected = [];
      this.implied_selected = [];
    }
  },
});
