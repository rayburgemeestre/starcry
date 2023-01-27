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
    >
    </q-table>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { storeToRefs } from 'pinia';
import { useViewpointStore } from 'stores/viewpoint';

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

let viewpoint_store = storeToRefs(useViewpointStore());

let rows = ref([]);
for (let v in viewpoint_store) {
  rows.value.push([v, viewpoint_store[v]]);
}

export default defineComponent({
  name: 'ViewpointComponent',
  props: {},
  setup() {
    return {
      rows,
      columns,
    };
  },
  computed: {},
});
</script>
