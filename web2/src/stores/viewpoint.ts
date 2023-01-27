import { defineStore } from 'pinia';

export const useViewpointStore = defineStore('viewpoint', {
  state: () => ({
    scale: 1,
    view_x: 0,
    view_y: 0,
    canvas_w: 1920,
    canvas_h: 1080,
    offset_x: 0,
    offset_y: 0,
  }),
});
