import { defineStore } from 'pinia';

export const useObjectsStore = defineStore('objects', {
  state: () => ({
    objects: [],
    selected: [],
  }),

  getters: {},

  actions: {
    addSelected(value) {
      if (!this.selected.includes(value)) {
        this.selected.push(value);
      }
    },
    removeSelected(value) {
      const index = this.selected.indexOf(value);
      if (index > -1) {
        this.selected.splice(index, 1);
      }
    },

    isSelected(value) {
      return this.selected.includes(value);
    },

    parentLookup(unique_id) {
      // TODO: this obviously needs to be persisted, but first let's see if it works.
      const lookup = {};
      const parentStack = []; // stack to keep track of parents at each level

      for (const obj of this.objects) {
        if (obj.level > 0) {
          // only non-root nodes have a parent
          lookup[obj.unique_id] = parentStack[obj.level - 1];
        }

        // update the current object as the potential parent for the next level
        parentStack[obj.level] = obj.unique_id;
      }
      return lookup[unique_id];
    },
  },
});
