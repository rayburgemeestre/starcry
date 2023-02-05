<template>
  <div>{{ objects_store.objects }}</div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { StarcryAPI } from 'components/api';
import { useObjectsStore } from 'stores/objects';
export default defineComponent({
  name: 'ObjectsPage',
  setup() {
    let objects_store = useObjectsStore();
    const objects_endpoint = new StarcryAPI(
      'objects',
      StarcryAPI.json_type,
      (msg) => {},
      (buffer) => {
        objects_store.objects = buffer;
        // this.render_objects();
      },
      (_) => {},
      (_) => {}
    );

    return {
      objects_store,
    };
  },
});
</script>
