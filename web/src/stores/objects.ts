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
    // selections explicitly made by user (e.g., by clicking on objects)
    user_selected: [] as number[],
    // parents of user selected items (they are "implied" selected)
    implied_selected: [] as number[],
    // overrides from the GUI (e.g., toggle expand/collapse)
    gui_implied_selected: [] as number[],
    // lookup parents
    lookup: {} as Record<number, number>,
    has_children: {} as Record<number, boolean>,
  }),
  getters: {},
  actions: {
    addGuiSelected(value: number) {
      if (!this.gui_implied_selected.includes(value)) {
        this.gui_implied_selected.push(value);
      }
    },
    removeGuiSelected(value: number) {
      const index = this.gui_implied_selected.indexOf(value);
      if (index > -1) {
        this.gui_implied_selected.splice(index, 1);
      }
    },
    isSelected(value: number) {
      if (!this.hasChildren(value)) {
        return 4;
      } else if (this.user_selected.includes(value)) {
        return 1;
      } else if (this.gui_implied_selected.includes(value)) {
        return 3;
      } else if (this.implied_selected.includes(value)) {
        return 2;
      }
      return 0;
    },
    isUserSelected(value: number) {
      return this.user_selected.includes(value);
    },
    hasParent(value: number) {
      return value in this.lookup;
    },
    hasChildren(value: number) {
      return value in this.has_children && this.has_children[value];
    },
    isSelectedArray(value: number[]) {
      for (const v of value) {
        if (this.isSelected(v) !== 1 && this.isSelected(v) !== 3) {
          return false;
        }
      }
      return true;
    },
    updateLookupTable() {
      const parentStack = []; // stack to keep track of parents at each level
      this.lookup = {};
      this.has_children = {};
      try {
        for (const obj of this.objects) {
          if (obj.level > 0) {
            // only non-root nodes have a parent
            this.lookup[obj.unique_id] = parentStack[obj.level - 1];
            this.has_children[parentStack[obj.level - 1]] = true;
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

      this.user_selected.push(unique_id);

      while (currentID !== undefined && this.lookup[currentID] !== undefined) {
        ids.push(currentID);
        currentID = this.lookup[currentID];
      }

      if (currentID !== undefined) {
        ids.push(currentID); // add the last id which doesn't have a parent in lookup
      }

      this.implied_selected = [...new Set([...this.implied_selected, ...ids])];
      this.gui_implied_selected = [
        ...new Set([...this.gui_implied_selected, ...ids]),
      ];
    },
    onUserDeSelected(unique_id: number) {
      // remove user first
      let index = this.user_selected.indexOf(unique_id);
      if (index > -1) {
        this.user_selected.splice(index, 1);
      }
      // also remove from gui selected
      index = this.gui_implied_selected.indexOf(unique_id);
      if (index > -1) {
        this.gui_implied_selected.splice(index, 1);
      }

      // update implied
      let ids: number[] = [];
      for (const v of this.user_selected) {
        const newids = this.parentsLookup(v);
        ids = [...new Set([...newids, ...ids])];
      }
      this.implied_selected = ids;
    },
    reset() {
      this.user_selected = [];
      this.gui_implied_selected = [];
      this.implied_selected = [];
    },
    reset_gui() {
      this.gui_implied_selected = [];
    },
  },
});
