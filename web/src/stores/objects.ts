import { defineStore } from 'pinia';
import { ref } from 'vue';

export type ObjectType = {
  unique_id: number;
  level: number;
  label: string;
  id: string;
  x: number;
  y: number;
  x2: number;
  y2: number;
  radius: number;
  '#': number;
  type: string;
};

export const useObjectsStore = defineStore('objects', {
  state: () => ({
    objects: [] as ObjectType[],
    // mainly focused object by user
    object_id: '',
    object: {} as ObjectType,
    // selections explicitly made by user (e.g., by clicking on objects)
    user_selected: [] as number[],
    // selections to show info by user
    user_selected_info: [] as number[],
    // parents of user selected items (they are "implied" selected)
    implied_selected: [] as number[],
    // overrides from the GUI (e.g., toggle expand/collapse)
    gui_implied_selected: [] as number[],
    // lookup parents
    lookup: {} as Record<number, number>,
    has_children: {} as Record<number, boolean>,
    children: {} as Record<number, number[]>,

    new_object_selected_by_user: ref(0),
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
    isUserInfoSelected(value: number) {
      return this.user_selected_info.includes(value);
    },
    hasParent(value: number) {
      return value in this.lookup;
    },
    hasChildren(value: number) {
      return value in this.has_children && this.has_children[value];
    },
    getChildren(value: number) {
      if (value in this.children) {
        return this.children[value];
      }
      return [];
    },
    getChildrenRecursive(value: number): number[] {
      const children = this.getChildren(value);
      const recursiveChildren: number[] = children.flatMap((child) => this.getChildrenRecursive(child));
      return [...children, ...recursiveChildren];
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
      const parentStack: number[] = []; // stack to keep track of parents at each level
      this.lookup = {};
      this.has_children = {};
      this.children = {};
      try {
        for (const obj of this.objects) {
          if (obj.level > 0) {
            // only non-root nodes have a parent
            const parent_id = parentStack[obj.level - 1];
            this.lookup[obj.unique_id] = parent_id;
            if (!(parent_id in this.children)) {
              this.children[parent_id] = [];
            }
            this.children[parent_id].push(obj.unique_id);
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
      this.gui_implied_selected = [...new Set([...this.gui_implied_selected, ...ids])];
    },
    onUserSelectedInfo(unique_id: number) {
      this.user_selected_info.push(unique_id);
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
    onUserInfoDeSelected(unique_id: number) {
      // remove user first
      const index = this.user_selected_info.indexOf(unique_id);
      if (index > -1) {
        this.user_selected_info.splice(index, 1);
      }
    },
    reset() {
      this.user_selected = [];
      this.user_selected_info = [];
      this.gui_implied_selected = [];
      this.implied_selected = [];
    },
    reset_gui() {
      this.gui_implied_selected = [];
    },
    set_selected(object_id: string, object: ObjectType) {
      console.log('RBU --- set_selected:', object);
      this.object_id = object_id;
      this.object = object;
      this.new_object_selected_by_user++;
    },
  },
});
