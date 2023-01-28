<template>
  <div class="q-pa-md">
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
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, watch } from 'vue';
import { storeToRefs } from 'pinia';
import { useViewpointStore } from 'stores/viewpoint';
import { StarcryAPI } from 'components/api';

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

let first_load = true;
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

export default defineComponent({
  name: 'ViewpointComponent',
  props: {},
  setup() {
    let scheduled_update = function () {
      let str = JSON.stringify({
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
      viewpoint_endpoint.send(str);

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

    return {
      rows,
      columns,
      viewpoint_store_raw,
    };
  },
  computed: {},
});
</script>
