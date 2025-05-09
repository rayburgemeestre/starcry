import { defineStore } from 'pinia';
import { ref } from 'vue';

export const useScriptStore = defineStore('script', {
  state: () => ({
    filename: '',
    script: '',
    current_function: '',
    snippet: '',
    result: '',
    frame: 0,

    video: {},
    video_spec: {},
    object_spec: {},
    preview: {},
    frames_per_scene: [] as number[],
    max_frames: 0,
    selected: [] as number[],
    highlighted: [] as number[],

    auto_render: true,
    num_chunks: 64,
    timeout: 20,

    texture_w: ref(0),
    texture_h: ref(0),

    job_skipped: ref(0),
    job_rendering: ref(0),
    job_rendered: ref(0),
    job_queued: ref(0),
    job_timedout: ref(0),

    // used as event handlers
    value_updated_by_user: ref(0),
    snippet_updated_by_user: ref(0),
    result_updated_by_user: ref(0),

    render_requested_by_user: ref(0), // ??
    render_requested_by_user_v2: ref(0),
    render_completed_by_server: ref(0),
    restart_server_requested_by_user: ref(0),
    texture_size_updated_by_server: ref(0),
    re_render_editor_sidepane: ref(0),
    request_video_spec_by_user: ref(0),
    request_video_spec_received: ref(0),
    request_object_spec_by_user: ref(0),
    request_object_spec_received: ref(0),
  }),

  getters: {},

  actions: {
    set_value(new_val: string, emit_event: boolean) {
      this.script = new_val;
      if (emit_event) this.value_updated_by_user++;
    },
    set_snippet(new_val: string, emit_event: boolean) {
      this.snippet = new_val;
      if (emit_event) this.snippet_updated_by_user++;
    },
    set_result(new_val: string, emit_event: boolean) {
      this.result = new_val;
      if (emit_event) this.result_updated_by_user++;
    },
    clearSelectedObjects() {
      this.selected = [];
    },
    addSelectedObject(unique_id: number) {
      this.selected.push(unique_id);
    },
    removeSelectedObject(unique_id: number) {
      const index = this.selected.indexOf(unique_id);
      if (index > -1) {
        this.selected.splice(index, 1);
      }
    },
    highlightObject(unique_id: number) {
      const index = this.highlighted.indexOf(unique_id);
      if (index === -1) {
        this.highlighted.push(unique_id);
      } else {
        this.highlighted.splice(index, 1);
      }
    },
    reset() {
      this.selected = [];
    },
  },
});
