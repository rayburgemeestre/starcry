<template>
  <div :key="componentKey">
    <div v-for="object in objects_store.objects" :key="object.id">
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
        <span
          class="col collapse"
          v-if="objects_store.isSelected(object['unique_id'])"
          @click="click(this, object.unique_id)"
          >▼</span
        >
        <span
          class="col expand"
          v-if="!objects_store.isSelected(object['unique_id'])"
          @click="click(this, object.unique_id)"
          >▶</span
        >
        <span class="col type">{{ to_utf8_symbol(object['type']) }}</span>
        <span class="col level">{{ object['level'] }}</span>
        <span class="col unique_id">{{ object['unique_id'] }}</span>
        <span class="col random_hash">{{ object['random_hash'] }}</span>
        <span class="col id">{{ object['id'] }}</span>
        <span class="col gradient">{{ object['gradient'] }}</span>
      </div>
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="objects_store.isSelected(object['unique_id'])"
        class="item"
      >
        <q-table
          :style="{
            marginLeft: 25 + object['level'] * 20 + 'px',
            marginTop: '5px',
          }"
          dense
          :rows="rows(object['unique_id'])"
          :columns="columns"
          row-key="name"
          :filter="filter"
          hide-header
          hide-pagination
          :rows-per-page-options="[100]"
        >
        </q-table>
      </div>
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
        objects_store.updateLookupTable();
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

    function rows(unique_id) {
      // get the object with matching unique_id, from the array
      let object = objects_store.objects.find(
        (element) => element.unique_id === unique_id
      );
      return Object.entries(object);
    }

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
        field: (row) => row[1],
        sortable: true,
      },
    ];

    return {
      objects_store,
      to_utf8_symbol,
      click,
      componentKey,
      rows,
      columns,
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

.col.expand,
.col.collapse {
  cursor: pointer;
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
.col.random_hash {
  max-width: 60px;
  padding: 0;
}
.item:hover .col {
  background-color: rgba(255, 255, 255, 0.28);
}
.col.expand:hover,
.col.collapse:hover {
  background-color: #990000;
}
</style>
