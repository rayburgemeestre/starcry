<template>
  <q-table
    style="height: 400px"
    flat
    bordered
    dense
    title="Objects"
    :rows="rows"
    :columns="columns"
    row-key="unique_id"
    virtual-scroll
    v-model:pagination="pagination"
    :rows-per-page-options="[0]"
    v-model:selected="selected"
    selection="multiple"
    @row-click="onRowClick"
  >
    <template v-slot:body="props">
      <q-tr :props="props" :style="`position: relative; left: ${props.row.level * 10}px;`">
        <q-td auto-width>
          <q-checkbox v-model="props.selected" />
        </q-td>
        <q-td v-for="col in props.cols" :key="col.name" :props="props">
          <template v-if="col.name === 'actions'">
            <q-btn color="primary" dense size="sm" label="isolate" @click.stop="onIsolateClick(props.row)" />
          </template>
          <template v-else>
            {{ col.value }}
          </template>
        </q-td>
      </q-tr>
    </template>
  </q-table>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { useObjectsStore } from 'stores/objects';
import { useScriptStore } from 'stores/script';

function to_utf8_symbol(type: string) {
  switch (type) {
    case 'circle':
      return '⭕';
    case 'line':
      return '⎯';
    case 'script':
      return '⚡';
    case 'text':
      return '💬';
    case 'ellipse':
      return '⬭';
    case 'none':
      return '❓';
  }
}

const selected = ref<any[]>([]);

export default defineComponent({
  name: 'ObjectBrowser2',
  components: {},
  computed: {
    rows() {
      let objects_store = useObjectsStore();
      let stack = [];
      let filtered_objects = [];

      // Get all filter stacks from selected items
      let filter_stacks = selected.value.map((item) => item.stack);

      for (let obj of objects_store.objects) {
        obj.actions = '--insert button here--';
        obj.type2 = to_utf8_symbol(obj.type);
        while (stack.length > obj.level + 1) {
          stack.pop();
        }
        while (stack.length < obj.level + 1) {
          stack.push(null);
        }
        stack[obj.level] = obj.unique_id;
        // Create a new array and copy values
        obj.stack = [...stack];

        // Check if object matches any of the filter stacks
        let matches_any_filter = false;

        if (filter_stacks.length === 0) {
          // When nothing is selected, only show root level objects
          matches_any_filter = obj.level === 0;
        } else {
          for (let filter_stack of filter_stacks) {
            let len_match = obj.stack.length === filter_stack.length + 1;
            let stack_starts_with_filter_stack = obj.stack
              .slice(0, filter_stack.length)
              .every((value, index) => value === filter_stack[index]);
            if ((len_match && stack_starts_with_filter_stack) || filter_stack.includes(obj.unique_id)) {
              matches_any_filter = true;
              break;
            }
          }
        }

        if (obj.level === 0 || matches_any_filter) {
          filtered_objects.push(obj);
        }
      }
      return filtered_objects;
    },
    columns() {
      let objects_store = useObjectsStore();
      let columns = [];
      for (let obj of objects_store.objects) {
        for (let key in obj) {
          columns.push({
            name: key,
            required: true,
            label: key,
            field: (row) => row[key],
            format: (val) => `${val}`,
            sortable: false,
          });
        }
        break; // just inspect first object is enough
      }
      // reorder columns a little bit, make sure the first column is the object name
      // sort that 'level', key is first, then 'id', then 'type'

      let ordering = ['unique_id', 'actions', 'level', 'id', 'type', 'type2', 'stack'];
      columns.sort((a, b) => {
        const aIndex = ordering.indexOf(a.name);
        const bIndex = ordering.indexOf(b.name);

        // If both are in ordering array, sort by their position
        if (aIndex >= 0 && bIndex >= 0) {
          return aIndex - bIndex;
        }

        // If only a is in ordering, it should come first
        if (aIndex >= 0) return -1;

        // If only b is in ordering, it should come first
        if (bIndex >= 0) return 1;

        // Neither in ordering, sort alphabetically
        return a.name.localeCompare(b.name);
      });

      return columns;
    },
  },
  setup() {
    //const componentKey = ref(0);
    let objects_store = useObjectsStore();
    let scripts_store = useScriptStore();
    const pagination = ref({
      rowsPerPage: 0,
    });

    const onIsolateClick = (row: any) => {
      console.log('TODO: isolate button clicked for row:', row);
    };

    const onRowClick = (evt: any, row: any) => {
      console.log('Selected row:', row);
      // Toggle selection for the clicked row
      const index = selected.value.findIndex((r) => r.unique_id === row.unique_id);
      if (index === -1) {
        // Add to selection if not already selected
        selected.value.push(row);
      } else {
        // Remove from selection if already selected
        selected.value.splice(index, 1);
      }
      console.log('selected!!: ', selected.value);
    };

    return {
      objects_store,
      scripts_store,
      to_utf8_symbol,
      //componentKey,
      selected,
      pagination,
      onRowClick,
      onIsolateClick,
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
.col.info {
  width: 60px;
  max-width: 60px;
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

/* Add styles for selected row */
:deep(.q-table__card) .q-table__row--selected {
  background: rgba(0, 0, 255, 0.1);
}

:deep(.q-table__card) .q-table__row:hover {
  background: rgba(0, 0, 255, 0.05);
}

:deep(.test-button) {
  padding: 2px 8px;
  background-color: #4a4a4a;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
}

:deep(.test-button:hover) {
  background-color: #666666;
}
</style>
