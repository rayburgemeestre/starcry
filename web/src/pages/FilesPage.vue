<template>
  <div class="q-pa-md q-gutter-sm">
    <q-input ref="filterRef" filled v-model="files.filter" label="Filter">
      <template v-slot:append>
        <q-icon v-if="files.filter !== ''" name="clear" class="cursor-pointer" @click="resetFilter()" />
      </template>
    </q-input>

    <q-tree
      :nodes="files.simple"
      node-key="key"
      label-key="label"
      :filter="files.filter"
      default-expand-all
      dense
      selected-color="primary"
      v-model:selected="files.selected"
    />
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { useFilesStore } from 'stores/files';

export default defineComponent({
  name: 'FilesPage',
  setup() {
    let files = useFilesStore();
    const filterRef = ref(null);

    return {
      files,
      filterRef,
      resetFilter() {
        files.resetFilter();
        filterRef.value.focus();
      },
    };
  },
});
</script>
