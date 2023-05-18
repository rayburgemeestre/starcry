<template>
  <div :key="componentKey">
    <div
      v-for="object in objects_store.objects"
      :key="object.id"
      @click="click(this, object.unique_id)"
    >
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="
          object['level'] === 0 ||
          objects_store.isSelected(
            objects_store.parentLookup(object['unique_id'])
          )
        "
        class="item"
      >
        <span class="col type">{{ to_utf8_symbol(object['type']) }}</span>
        <span class="col level">{{ object['level'] }}</span>
        <span class="col unique_id">{{ object['unique_id'] }}</span>
        <span class="col id">{{ object['id'] }}</span>
        <span class="col gradient">{{ object['gradient'] }}</span>
      </div>
      <!--<span>{{ object }}</span> -->
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { useObjectsStore } from 'stores/objects';
import { StarcryAPI } from 'components/api';
export default defineComponent({
  // name: 'ComponentName'
  name: 'ObjectBrowser',
  components: {},
  setup() {
    const componentKey = ref(0);
    let objects_store = useObjectsStore();
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    const objects_endpoint = new StarcryAPI(
      'objects',
      StarcryAPI.json_type,
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (msg) => {},
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      (buffer) => {
        objects_store.objects = buffer;
      },
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (_) => {},
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (_) => {}
    );

    function to_utf8_symbol(type) {
      switch (type) {
        case 'circle':
          return '●';
        case 'line':
          return '⎯';
        case 'script':
          return '⚡';
        case 'text':
          return 'T';
        case 'ellipse':
          return '⬭';
        case 'none':
          return '?';
      }
    }

    function click(source_elem, unique_id) {
      if (objects_store.isSelected(unique_id)) {
        objects_store.removeSelected(unique_id);
        return;
      }
      objects_store.addSelected(unique_id);
    }

    return {
      objects_store,
      to_utf8_symbol,
      click,
      componentKey,
    };
  },
});
</script>

<style scoped>
.item {
  display: flex;
  flex-direction: row; /* not necessary because row is the default direction */
  flex-wrap: nowrap;
}
.item:first-child {
  margin-top: 10px;
}
.col {
  background-color: #333333;
  margin-right: 5px;
  padding-left: 2px;
  padding-right: 2px;
}
.col.type {
  max-width: 10px;
  margin-left: 10px;
  padding: 0;
}
.col.level {
  max-width: 10px;
  padding: 0;
}
.col.unique_id {
  max-width: 60px;
  padding: 0;
}
.item:hover .col {
  background-color: rgba(255, 255, 255, 0.28);
}
</style>
