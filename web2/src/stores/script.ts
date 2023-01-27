import { defineStore } from 'pinia';

export const useScriptStore = defineStore('script', {
  state: () => ({
    filename: 'input/test.js',
    script: '',
    frame: 1,

    video: {},
    preview: {},
    frames_per_scene: [],
    max_frames: 0,

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
