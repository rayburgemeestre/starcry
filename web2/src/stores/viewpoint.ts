import { defineStore } from 'pinia';
import { ref } from 'vue';

export const useViewpointStore = defineStore('viewpoint', {
  state: () => ({
    scale: 1,
    view_x: 0,
    view_y: 0,
    canvas_w: 1920,
    canvas_h: 1080,
    offset_x: 0,
    offset_y: 0,
    script_hash: '',

    raw: false,
    preview: false,
    labels: false,
    caching: false,
    debug: false,
    save: false,

    viewpoint_updated_by_client: ref(0),
  }),
  actions: {
    select() {
      const view_x = this.view_x - this.canvas_w / 2;
      const view_y = this.view_y - this.canvas_h / 2;
      this.offset_x += view_x / this.scale;
      this.offset_y += view_y / this.scale;
    },
    reset() {
      this.scale = 1;
      this.offset_x = 0;
      this.offset_y = 0;
      this.raw = false;
      this.preview = false;
      this.labels = false;
      this.caching = false;
      this.debug = false;
      this.save = false;
    },
  },
});
