<template>
  <div>My component
    <div class="item" v-for="object in objects_store.objects" :key="object.id">
      <span class="col">{{ to_utf8_symbol(object['type']) }}</span>
      <span class="col">{{ object['level'] }}</span>
      <span class="col">{{ object['unique_id'] }}</span>
      <span class="col">{{ object['id'] }}</span>
      <span class="col">{{ object['gradient'] }}</span>
      <!--<span>{{ object }}</span> -->
    </div>
  </div>
</template>

<script lang="ts">
import {defineComponent} from 'vue'
import {useObjectsStore} from 'stores/objects';
import {StarcryAPI} from 'components/api';
export default defineComponent({
  // name: 'ComponentName'
  name: 'ObjectBrowser',
  components: {},
  setup() {
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

    return {
      objects_store,
      to_utf8_symbol,
    };
  },
})
</script>

<style scoped>
 .item {
   display: flex;
   flex-direction: row; /* not necessary because row is the default direction */
   flex-wrap: nowrap;
 }
 .col {
   background-color: #c0c0c0;
   margin-right: 5px;
   padding-left: 2px;
   padding-right: 2px;
 }
</style>
