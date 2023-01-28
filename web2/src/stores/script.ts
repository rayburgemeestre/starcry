import { defineStore } from 'pinia';
import { ref } from 'vue';

export const useScriptStore = defineStore('script', {
  state: () => ({
    filename: 'input/test.js',
    script: '',
    frame: 2, // temporary hack until we fix server-side initialization + spawn issue

    video: {},
    preview: {},
    frames_per_scene: [],
    max_frames: 0,

    auto_render: true,

    // used as event handlers
    value_updated_by_user: ref(0),
    render_requested_by_user: ref(0),
  }),

  getters: {},

  actions: {
    set_value(new_val: string) {
      this.script = new_val;
      this.value_updated_by_user++;
    },
  },
});
