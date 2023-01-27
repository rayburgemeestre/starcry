import { defineStore } from 'pinia';

export const useScriptStore = defineStore('script', {
  state: () => ({
    filename: 'input/test.js',
    script: '',

    // used as event handler
    value_updated_by_user: 0,
  }),

  getters: {},

  actions: {
    set_value(new_val: string) {
      this.script = new_val;
      this.value_updated_by_user++;
    },
  },
});
