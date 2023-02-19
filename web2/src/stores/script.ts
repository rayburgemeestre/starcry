import { defineStore } from 'pinia';
import { ref } from 'vue';

export const useScriptStore = defineStore('script', {
  state: () => ({
    filename: 'input/test.js',
    script: '',
    frame: 2, // temporary hack until we fix server-side initialization + spawn issue

    video: {},
    preview: {},
    frames_per_scene: [] as number[],
    max_frames: 0,

    auto_render: true,

    texture_w: ref(0),
    texture_h: ref(0),

    // used as event handlers
    value_updated_by_user: ref(0),
    render_requested_by_user: ref(0), // ??
    render_requested_by_user_v2: ref(0),
    render_completed_by_server: ref(0),
    texture_size_updated_by_server: ref(0),
  }),

  getters: {},

  actions: {
    set_value(new_val: string) {
      this.script = new_val;
      this.value_updated_by_user++;
    },
  },
});