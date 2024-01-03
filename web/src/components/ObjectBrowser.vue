<template>
  <div class="q-pa-md">
    <br />
    <q-btn color="secondary" style="width: 33.33%" @click="reset()"
      >Reset</q-btn
    >
    <q-btn
      color="secondary"
      style="width: 33.33%"
      @click="toggle_expand(true)"
      v-if="state === 'expand'"
      >Collapse</q-btn
    >
    <q-btn
      color="secondary"
      style="width: 33.33%"
      @click="toggle_expand(false)"
      v-if="state !== 'expand'"
      >Expand</q-btn
    >
    <q-btn color="secondary" style="width: 33.33%" @click="filtering()"
      >Filter</q-btn
    >
  </div>

  <div :key="componentKey">
    state: {{ state }}, filter: {{ filter_selected }}
    <div v-for="object in objects_store.objects" :key="object.id">
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="object['level'] === 0 || show_object(object['unique_id'])"
        class="item"
      >
        <span
          class="col collapse"
          v-if="show_icon(object['unique_id'], 1)"
          @click="expand_icon_click(this, object.unique_id)"
          >➔</span
        >
        <span
          class="col collapse"
          v-if="show_icon(object['unique_id'], 2)"
          @click="expand_icon_click(this, object.unique_id)"
          >➘</span
        >
        <span
          class="col collapse"
          v-if="show_icon(object['unique_id'], 3)"
          @click="expand_icon_click(this, object.unique_id)"
          >➘➘</span
        >
        <span
          class="col expand"
          v-if="show_icon(object['unique_id'], 0)"
          @click="expand_icon_click(this, object.unique_id)"
          >&nbsp;</span
        >
        <span
          class="col type"
          :style="{
            backgroundColor: objects_store.isUserSelected(object['unique_id'])
              ? 'red'
              : '',
            color: objects_store.isUserSelected(object['unique_id'])
              ? 'black'
              : '',
          }"
          @click="user_select(this, object.unique_id)"
          >focus<!-- {{ to_utf8_symbol(object['type']) }}--></span
        >
        <span
          class="col unique_id"
          :style="{
            backgroundColor: scripts_store.highlighted.includes(
              object['unique_id']
            )
              ? 'cyan'
              : '',
            color: scripts_store.highlighted.includes(object['unique_id'])
              ? 'black'
              : '',
          }"
          @click="highlight(this, object.unique_id)"
          >{{ object['unique_id'] }}</span
        >
        <span class="col id">{{ object['id'] }}</span>
        <span class="col gradient">{{ object['gradient'] }}</span>
      </div>
      <div
        :style="{ marginLeft: object['level'] * 20 + 'px' }"
        v-if="
          objects_store.isUserSelected(object['unique_id']) &&
          show_object(object['unique_id'])
        "
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

export default defineComponent({
  // name: 'ComponentName'
  name: 'ObjectBrowser',
  components: {},
  setup() {
    const componentKey = ref(0);
    let objects_store = useObjectsStore();
    let scripts_store = useScriptStore();
    let state = ref('');
    let filter_selected = ref(false);

    function to_utf8_symbol(type) {
      switch (type) {
        case 'circle':
          return '◯';
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

    function expand_icon_click(source_elem, unique_id) {
      const selected = objects_store.isSelected(unique_id);
      if (selected === 1) {
        return;
      } else if (selected === 2) {
        objects_store.addGuiSelected(unique_id);
        return;
      } else if (selected === 3) {
        objects_store.removeGuiSelected(unique_id);
        return;
      }
      objects_store.addGuiSelected(unique_id);
    }

    function highlight(source_elem, unique_id) {
      scripts_store.highlightObject(unique_id);
      scripts_store.render_completed_by_server++;
    }

    function user_select(source_elem, unique_id) {
      if (objects_store.isUserSelected(unique_id)) {
        scripts_store.removeSelectedObject(unique_id);
        objects_store.onUserDeSelected(unique_id);
      } else {
        scripts_store.addSelectedObject(unique_id);
        objects_store.onUserSelected(unique_id);
      }
    }

    function reset() {
      objects_store.reset();
      scripts_store.reset();
      return;
    }

    function toggle_expand(collapse: boolean) {
      if (collapse) {
        objects_store.reset_gui();
        state.value = '';
      } else {
        state.value = 'expand';
      }
    }

    function filtering() {
      filter_selected.value = !filter_selected.value;
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

    function show_object(unique_id: number) {
      if (filter_selected.value) {
        return objects_store.isSelected(unique_id) !== 0;
      }
      return (
        objects_store.isSelectedArray(objects_store.parentsLookup(unique_id)) ||
        state.value === 'expand'
      );
    }

    function show_icon(unique_id: number, expected: number) {
      return objects_store.isSelected(unique_id) === expected;
    }

    return {
      objects_store,
      scripts_store,
      to_utf8_symbol,
      expand_icon_click,
      highlight,
      user_select,
      componentKey,
      rows,
      columns,
      reset,
      toggle_expand,
      state,
      filtering,
      filter_selected,
      show_object,
      show_icon,
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
  max-width: 35px;
  margin-left: 10px;
  padding: 0;
}
.col.unique_id {
  width: 60px;
  max-width: 60px;
  padding: 0;
}
.col.id {
  width: 60px;
  max-width: 60px;
  padding: 0;
}
.col.gradient {
  width: 100px;
  max-width: 100px;
}
.item:hover .col {
  background-color: rgba(255, 255, 255, 0.28);
}
.col.expand,
.col.collapse {
  margin-left: 10px;
  width: 15px;
  max-width: 15px;
}
.col.expand:hover,
.col.collapse:hover {
  background-color: #990000;
}
</style>
