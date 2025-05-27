import { defineStore } from 'pinia';
import { TreeNode } from 'src/core/filetree';

export const useFilesStore = defineStore('files', {
  state: () => ({
    filter: '',
    // ??? filterRef: null,
    selected: '',
    simple: [] as TreeNode[],
  }),

  getters: {},

  actions: {
    resetFilter() {
      this.filter = '';
      // this.filterRef.focus();
    },
  },
});
