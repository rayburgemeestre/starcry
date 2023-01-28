<template>
  <div class="q-pa-md">
    <br />
    <q-btn
      :loading="loading"
      color="secondary"
      @click="render_current_frame"
      style="width: 100%"
      >Render</q-btn
    >
    <br />
    <br />
    <q-btn color="secondary" style="width: 50%">Previous</q-btn>
    <q-btn color="secondary" style="width: 50%">Next</q-btn>

    <q-table
      dense
      :rows="rows"
      :columns="columns"
      row-key="name"
      :filter="filter"
      hide-header
      hide-pagination
      :rows-per-page-options="[100]"
    >
    </q-table>
    <br />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.labels"
      label="Labels"
      color="#990000"
    />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.debug"
      label="Debug"
      color="#990000"
    />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.raw"
      label="Raw"
      color="#990000"
    />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.preview"
      label="Preview"
      color="#990000"
    />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.caching"
      label="Caching"
      color="#990000"
    />
    <q-checkbox
      dark
      v-model="viewpoint_store_raw.save"
      label="Save"
      color="#990000"
    />
    <br />
    <q-btn color="secondary" @click="reset_values" style="width: 100%"
      >Reset</q-btn
    >
    <q-checkbox
      dark
      v-model="script_store.auto_render"
      label="Auto Render"
      color="#990000"
    />
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, watch } from 'vue';
import { storeToRefs } from 'pinia';
import { useViewpointStore } from 'stores/viewpoint';
import { StarcryAPI } from 'components/api';
import { useScriptStore } from 'stores/script';

const columns = [
  {
    name: 'property',
    align: 'left',
    field: (row) => row[0],
    format: (val) => `${val}`,
    sortable: true,
  },
  {
    name: 'value',
    align: 'left',
    field: (row) => row[1].value,
    sortable: true,
  },
];

let viewpoint_store_raw = useViewpointStore();
let viewpoint_store = storeToRefs(useViewpointStore());
let script_store = useScriptStore();
let script_store_refs = storeToRefs(useScriptStore());
let loading = ref(false);

let first_load = true;

let bitmap_endpoint = new StarcryAPI(
  'bitmap',
  StarcryAPI.binary_type,
  (msg) => {
    // this.$data.websock_status = msg;
  },
  // eslint-disable-next-line @typescript-eslint/no-empty-function
  (buffer) => {},
  (_) => {
    // this.log('DEBUG', 'bitmap', 'websocket connected', '');
    // this.$data.connected_bitmap = true;
  },
  (_) => {
    // this.log('DEBUG', 'bitmap', 'websocket disconnected', '');
    // this.$data.connected_bitmap = false;
  }
);

let viewpoint_endpoint = new StarcryAPI(
  'viewpoint',
  StarcryAPI.json_type,
  // eslint-disable-next-line @typescript-eslint/no-empty-function
  (msg) => {},
  (buffer) => {
    viewpoint_store.scale.value = buffer['scale'];
    // this.$data.view_x = buffer["offset_x"] / buffer["scale"];
    // this.$data.view_y = buffer["offset_y"] / buffer["scale"];
    // this.$data.offsetX = buffer["offset_x"];
    // this.$data.offsetY = buffer["offset_y"];
    viewpoint_store.raw.value = buffer['raw'];
    viewpoint_store.preview.value = buffer['preview'];
    viewpoint_store.save.value = buffer['save'];
    viewpoint_store.labels.value = buffer['labels'];
    viewpoint_store.caching.value = buffer['caching'];
    viewpoint_store.debug.value = buffer['debug'];
  },
  (_) => {
    if (first_load)
      viewpoint_endpoint.send(
        JSON.stringify({
          operation: 'read',
        })
      );
    /*else this.scheduled_update();
     */
    first_load = false;
  }
);

let rows = ref([]);
for (let v in viewpoint_store) {
  // we're already creating checkboxes for booleans manually
  if (typeof viewpoint_store[v].value !== 'boolean')
    rows.value.push([v, viewpoint_store[v]]);
}

let timer: NodeJS.Timeout | null = null;

function serialize_viewpoint() {
  return JSON.stringify({
    operation: 'set',
    scale: viewpoint_store.scale.value,
    offset_x: viewpoint_store.offset_x.value,
    offset_y: viewpoint_store.offset_y.value,
    raw: viewpoint_store.raw.value,
    preview: viewpoint_store.preview.value,
    labels: viewpoint_store.labels.value,
    caching: viewpoint_store.caching.value,
    debug: !!viewpoint_store.debug.value,
    save: viewpoint_store.save.value,
    canvas_w: viewpoint_store.canvas_w.value,
    canvas_h: viewpoint_store.canvas_h.value,
  });
}

let previous_hash: string = serialize_viewpoint();

export default defineComponent({
  name: 'ViewpointComponent',
  props: {},
  setup() {
    let scheduled_update = function () {
      let str = serialize_viewpoint();
      viewpoint_endpoint.send(str);

      script_store.render_requested_by_user++;

      // // communicate offset_x and offset_y back to parent.
      // this.$parent.set_offset(this.$data.offsetX, this.$data.offsetY);
      //
      // if (this.$data.scale === this.$data.previous_scale) return;
      // // Delay might not be needed, just want to be sure the viewpoint send above is received before the render request.
      // if (this.$data.auto_render) {
      //   setTimeout(
      //     function () {
      //       this.$parent.set_frame();
      //     }.bind(this),
      //     10
      //   );
      // }
      // this.$data.previous_scale = this.$data.scale;
    };

    for (let v in viewpoint_store_raw.$state) {
      if (v === 'view_x' || v === 'view_y') continue;
      watch(
        () => viewpoint_store[v].value,
        (n) => {
          console.log('viewpoint_store_raw.$state changed', n, 'v', v);
          if (timer !== null) {
            clearTimeout(timer);
            timer = null;
          }
          timer = setTimeout(scheduled_update, 100);
        }
      );
    }

    const obj = {
      loading,
      rows,
      columns,
      viewpoint_store_raw,

      script_store,
      reset_values: function () {
        viewpoint_store_raw.reset();
      },
      render_current_frame: function () {
        previous_hash = serialize_viewpoint();
        loading.value = true;
        bitmap_endpoint.send(
          JSON.stringify({
            filename: script_store.filename,
            // frame: current_frame.value,
            // random frame between 1 and 150
            frame: parseInt(script_store.frame),
            // viewpoint_settings: viewpoint_settings,
            num_chunks: 1,
          })
        );
      },
    };
    // watch render_requested_by_user
    // obj.render_current_frame()
    let timer: NodeJS.Timer | null = null;
    watch(
      () => script_store.render_requested_by_user,
      (n) => {
        if (!script_store.auto_render) return;
        if (previous_hash === serialize_viewpoint()) {
          return;
        } // nothing new
        if (timer !== null) {
          clearTimeout(timer);
          timer = null;
        }
        timer = setTimeout(obj.render_current_frame, 100);
      }
    );
    return obj;
  },
  mounted() {
    bitmap_endpoint.on_message = (buffer) => {
      window.Module.last_buffer = buffer;
      window.Module.set_texture(buffer);
      // this.$data.rendering--;
      // this.process_queue();
      loading.value = false;
    };
  },
});
</script>
