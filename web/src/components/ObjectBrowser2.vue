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
    @row-click="onRowClick"
  >
    <template v-slot:body="props">
      <q-tr :props="props" :style="`position: relative; left: ${props.row.level * 10}px;`">
        <q-td v-for="col in props.cols" :key="col.name" :props="props">
          <template v-if="col.name === 'tree'">
            <div class="row q-gutter-sm">
              <q-btn
                v-if="props.row.has_children"
                :color="isExpanded(props.row) ? 'expanded' : 'collapsed'"
                dense
                size="sm"
                :label="isExpanded(props.row) ? 'collapse' : 'expand'"
                @click.stop="onExpandClick(props.row)"
              />
              <q-btn v-if="!props.row.has_children" dense size="sm" label="----->" />
            </div>
          </template>
          <template v-if="col.name === 'actions'">
            <div class="row q-gutter-sm">
              <q-btn color="primary" dense size="sm" label="isolate" @click.stop="onIsolateClick(props.row)" />
            </div>
          </template>
          <template v-else>
            {{ col.value }}
          </template>
        </q-td>
      </q-tr>
    </template>
  </q-table>

  <div v-if="selectedObject" class="isolated-object-container">
    <q-card flat bordered class="isolated-object-card">
      <q-card-section class="row items-center">
        <div class="text-h6">Isolated Object Details</div>
        <q-space />
        <q-btn color="negative" dense size="sm" label="Close" @click="onCloseClick" />
      </q-card-section>
      <q-separator />
      <q-card-section style="max-height: 300px" class="scroll">
        <q-list dense>
          <q-item v-for="(value, key) in objectProperties" :key="key">
            <q-item-section style="max-width: 200px">
              <q-item-label class="text-weight-bold">{{ key }}</q-item-label>
            </q-item-section>
            <q-item-section>
              <q-input v-model="editedObject[key]" dense outlined @change="updateObjectProperty(key)" />
            </q-item-section>
          </q-item>
        </q-list>
      </q-card-section>
    </q-card>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, computed } from 'vue';
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
    default:
      return '❓';
  }
}

export default defineComponent({
  name: 'ObjectBrowser2',
  setup() {
    const objects_store = useObjectsStore();
    const script_store = useScriptStore();

    interface TableObject {
      unique_id: number;
      id: string;
      type: string;
      level: number;
      has_children?: boolean;
      tree?: string;
      actions?: string;
      type2?: string;
      stack?: Array<number | null>;
      [key: string]: any; // For other properties
    }

    const expanded = ref<TableObject[]>([]);
    const selectedObject = ref<TableObject | null>(null);
    const pagination = ref({
      rowsPerPage: 0,
    });

    const rows = computed(() => {
      let stack: Array<number | null> = [];
      let filtered_objects: Array<TableObject> = [];

      // Get all filter stacks from selected items
      let filter_stacks = expanded.value.map((item) => item.stack);

      for (let obj of objects_store.objects) {
        const objAny = obj as TableObject;
        // button will be inserted in below two columns
        objAny.tree = '';
        objAny.actions = '';
        objAny.type2 = to_utf8_symbol(obj.type);
        while (stack.length > obj.level + 1) {
          stack.pop();
        }
        while (stack.length < obj.level + 1) {
          stack.push(null);
        }
        stack[obj.level] = obj.unique_id;
        // Create a new array and copy values
        objAny.stack = [...stack];

        // Check if object matches any of the filter stacks
        let matches_any_filter = false;

        if (filter_stacks.length === 0) {
          // When nothing is selected, only show root level objects
          matches_any_filter = obj.level === 0;
        } else {
          for (let filter_stack of filter_stacks) {
            let len_match = objAny.stack.length === filter_stack.length + 1;
            let stack_starts_with_filter_stack = objAny.stack
              .slice(0, filter_stack.length)
              .every((value: number | null, index: number) => value === filter_stack[index]);
            if ((len_match && stack_starts_with_filter_stack) || filter_stack.includes(obj.unique_id)) {
              matches_any_filter = true;
              break;
            }
          }
        }

        if (obj.level === 0 || matches_any_filter) {
          filtered_objects.push(objAny);
        }
      }
      return filtered_objects;
    });

    interface TableColumn {
      name: string;
      required: boolean;
      label: string;
      field: string | ((row: TableObject) => any);
      format?: (val: any) => string;
      sortable: boolean;
    }

    const columns = computed(() => {
      let cols: TableColumn[] = [];
      // Only process if there are objects
      if (objects_store.objects.length > 0) {
        const obj = objects_store.objects[0];
        for (let key in obj) {
          cols.push({
            name: key,
            required: true,
            label: key,
            field: (row: TableObject) => row[key],
            format: (val: any) => `${val}`,
            sortable: false,
          });
        }
      }

      // Add our custom columns
      if (!cols.find((col) => col.name === 'tree')) {
        cols.push({
          name: 'tree',
          required: true,
          label: 'Tree',
          field: 'tree',
          sortable: false,
        });
      }

      if (!cols.find((col) => col.name === 'actions')) {
        cols.push({
          name: 'actions',
          required: true,
          label: 'Actions',
          field: 'actions',
          sortable: false,
        });
      }

      if (!cols.find((col) => col.name === 'type2')) {
        cols.push({
          name: 'type2',
          required: true,
          label: 'Symbol',
          field: 'type2',
          sortable: false,
        });
      }

      // Reorder columns
      let ordering = [
        'tree',
        'unique_id',
        'actions',
        'level',
        'id',
        'type',
        'type2',
        'stack',
        'x',
        'y',
        'z',
        'r',
        'g',
        'b',
        'a',
      ];
      cols.sort((a, b) => {
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

      return cols;
    });

    const editedObject = ref<Record<string, any>>({});

    const objectProperties = computed(() => {
      if (!selectedObject.value) return {};

      // Filter out utility properties we don't want to show
      const excludeProperties = ['tree', 'actions', 'stack'];
      const result: Record<string, any> = {};

      // Order important properties first
      const orderingDetail = ['unique_id', 'id', 'type', 'level', 'x', 'y', 'z', 'r', 'g', 'b', 'a'];

      // First add ordered properties
      for (const prop of orderingDetail) {
        if (prop in selectedObject.value && !excludeProperties.includes(prop)) {
          result[prop] = selectedObject.value[prop];
        }
      }

      // Then add remaining properties
      for (const [key, value] of Object.entries(selectedObject.value)) {
        if (!excludeProperties.includes(key) && !orderingDetail.includes(key)) {
          result[key] = value;
        }
      }

      return result;
    });

    // Functions
    const isExpanded = (row: TableObject) => {
      return expanded.value.some((r) => r.unique_id === row.unique_id);
    };

    const onIsolateClick = (row: TableObject) => {
      // Create a clean copy of the row without the extra properties
      const cleanRow = { ...row };
      // Keep the original object for reference
      selectedObject.value = cleanRow;
      // Initialize the edited object with current values
      editedObject.value = { ...cleanRow };
      console.log('Isolated object:', cleanRow);
      script_store.clearSelectedObjects();
      script_store.addSelectedObject(cleanRow['unique_id']);
      for (const child of objects_store.getChildrenRecursive(cleanRow['unique_id'])) {
        script_store.addSelectedObject(child);
      }
      if (script_store.auto_render) {
        script_store.render_requested_by_user_v2++;
      }
    };

    const onCloseClick = () => {
      selectedObject.value = null;
      script_store.clearSelectedObjects();
      if (script_store.auto_render) {
        script_store.render_requested_by_user_v2++;
      }
    };

    const updateObjectProperty = (key: string) => {
      if (selectedObject.value && key in selectedObject.value) {
        // Update the original object with the edited value
        selectedObject.value[key] = editedObject.value[key];
        console.log(`Updated property ${key}:`, editedObject.value[key]);
      }
    };

    const onExpandClick = (row: TableObject) => {
      const index = expanded.value.findIndex((r) => r.unique_id === row.unique_id);
      if (index === -1) {
        // Add to selection if not already selected
        expanded.value.push(row);
      } else {
        // Remove from selection if already selected
        expanded.value.splice(index, 1);
      }
      console.log('expanded:', expanded.value);
    };

    const onRowClick = (_evt: Event, row: TableObject) => {
      // We can optionally highlight the row without isolating it
      console.log('Selected row:', row);
      // Another option is to also isolate the object on row click if that's desired
      // selectedObject.value = {...row};
    };

    return {
      // State
      expanded,
      selectedObject,
      pagination,
      editedObject,

      // Computed
      rows,
      columns,
      objectProperties,

      // Methods
      isExpanded,
      onIsolateClick,
      onCloseClick,
      onExpandClick,
      onRowClick,
      updateObjectProperty,
      to_utf8_symbol,
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

.isolated-object-container {
  margin-top: 20px;
}
.isolated-object-card {
  width: 100%;
}
.scroll {
  overflow-y: auto;
}
</style>
