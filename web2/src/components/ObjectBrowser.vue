<template>
  <div class="q-pa-md">
    <br />
    <q-btn color="secondary" style="width: 33.33%" @click="reset()">Reset</q-btn>
    <q-btn color="secondary" style="width: 33.33%" @click="collapse()">Collapse</q-btn>
    <q-btn color="secondary" style="width: 33.33%" @click="expand()">Expand</q-btn>
  </div>

  <div :key="componentKey">
    <div v-for="object in objects_store.objects" :key="object.id">
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="
          object['level'] === 0 ||
          objects_store.isSelectedArray(
            objects_store.parentsLookup(object['unique_id'])
          )
        "
        class="item"
      >
        <span
          class="col collapse"
          v-if="objects_store.isSelected(object['unique_id']) === 1"
          @click="click(this, object.unique_id)"
        >➔</span>
        <span
          class="col collapse"
          v-if="objects_store.isSelected(object['unique_id']) === 2"
          @click="click(this, object.unique_id)"
        >➘</span>
        <span
          class="col expand"
          v-if="!objects_store.isSelected(object['unique_id']) > 0"
          @click="click(this, object.unique_id)"
          >&nbsp;</span>
        <span class="col type">{{ to_utf8_symbol(object['type']) }}</span>
        <span class="col unique_id"
              :style="{ backgroundColor: scripts_store.highlighted.includes(object['unique_id']) ? 'cyan' : '',
                        color: scripts_store.highlighted.includes(object['unique_id']) ? 'black' : ''
                      }"
              @click="highlight(this, object.unique_id)">{{ object['unique_id'] }}</span>
        <span class="col id">{{ object['id'] }}</span>
        <span class="col gradient">{{ object['gradient'] }}</span>
      </div>
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="objects_store.isSelected(object['unique_id']) === 1 &&
          objects_store.isSelectedArray(
            objects_store.parentsLookup(object['unique_id'])
          )"
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
import { useScriptStore } from 'stores/script';
import { StarcryAPI } from 'components/api';
export default defineComponent({
  // name: 'ComponentName'
  name: 'ObjectBrowser',
  components: {},
  setup() {
    const componentKey = ref(0);
    let objects_store = useObjectsStore();
    let scripts_store = useScriptStore();

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
      const selected = objects_store.isSelected(unique_id);
      if (selected === 1) {
        return;
      } else if (selected === 2) {
        objects_store.removeImpliedSelected(unique_id);
        return;
      }
      objects_store.addImpliedSelected(unique_id);
    }

    function highlight(source_elem, unique_id) {
      scripts_store.highlightObject(unique_id);
      scripts_store.render_completed_by_server++;
    }

    function select(source_elem, unique_id) {
      scripts_store.addSelectedObject(unique_id);
    }

    function reset() {
      objects_store.reset();
      scripts_store.reset();
      return;
    }

    function collapse() {
      return;
    }

    function expand() {
      return;
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
      scripts_store,
      to_utf8_symbol,
      click,
      highlight,
      select,
      componentKey,
      rows,
      columns,
      reset,
      collapse,
      expand,
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
  font-family: monospace;
  font-size: 0.8em;
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
.col.unique_id {
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
