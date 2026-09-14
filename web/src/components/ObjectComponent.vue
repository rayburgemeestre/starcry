<template>
  <div>
    <q-item
      tag="label"
      v-ripple
      v-for="(object, key) in objects"
      :key="key"
      @click="
        if (selected != key) clearFunSelect();
        selected = key;
        $emit('object-selected', key);
      "
    >
      <q-item-section>
        <q-item-label>{{ key }}</q-item-label>
        <q-item-label caption> {{ object.type }} </q-item-label>
        <q-input v-if="selected == key" v-model="objectNames[key]" label="name" />
        <div class="row items-center" v-if="selected == key">
          <q-item-label header>Object Properties</q-item-label>
          <q-btn flat dense icon="add" @click="addObjectProperty" style="margin-top: -10px" />
        </div>
        <q-item v-if="objectAddingProperty">
          <q-select v-model="objectProperty" :options="objectProperties" label="Object Property" class="full-width" />
        </q-item>
        <q-item v-if="objectAddingProperty">
          <q-btn flat dense @click="addObjectProperty2" style="margin-top: -10px"> Add Object Property </q-btn>
        </q-item>
        <q-table
          v-if="selected == key"
          :rows="rows[key]"
          :columns="columns"
          row-key="name"
          @row-click="onRowClick"
          dense
          hide-header
          hide-pagination
          :rows-per-page-options="[100]"
        >
          <template v-slot:body-cell="props">
            <q-td :props="props" dense>
              <q-input
                v-if="
                  props.col.name === 'value' &&
                  props.row[0] != 'init' &&
                  props.row[0] != 'time' &&
                  editingCell === props.row[0]
                "
                :model-value="formatCellValue(props.row[1])"
                @update:model-value="updateCellValue($event, props.row)"
                dense
                borderless
                @blur="onCellChange(props.row)"
                @keyup.enter="onCellChange(props.row)"
              />
              <div
                v-if="
                  props.col.name === 'value' &&
                  props.row[0] != 'init' &&
                  props.row[0] != 'time' &&
                  editingCell !== props.row[0]
                "
                @click="startEditing($event.target, props.row)"
              >
                {{ typeof props.value === 'object' ? JSON.stringify(props.value) : props.value }}
              </div>
              <span v-if="props.col.name !== 'value' || props.row[0] == 'init' || props.row[0] == 'time'">
                {{ props.value }}
              </span>
            </q-td>
          </template>
        </q-table>
      </q-item-section>
    </q-item>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, shallowRef, watch, computed, PropType } from 'vue';
import { JsonValue } from 'src/types/types';
import { useScriptStore } from 'stores/script';
import { useProjectStore } from 'stores/project';

// Values come from json_parser, which turns the script into plain JSON: functions are
// replaced by "FUNCTION <n>" placeholder strings and resolved through parser.fun(), so
// everything reaching this component really is a JSON value.
interface ObjectData {
  [key: string]: JsonValue;
  type: string;
}

export default defineComponent({
  name: 'ObjectComponent',
  props: {
    objects: {
      type: Object as PropType<Record<string, ObjectData>>,
      required: true,
    },
    initialSelected: {
      type: String,
      default: '',
    },
  },
  emits: ['update:objects', 'object-selected', 'save-changes'],
  setup(props, { emit }) {
    const scriptStore = useScriptStore();
    const projectStore = useProjectStore();

    const selected = ref(props.initialSelected);
    const objectNames = ref<Record<string, string>>({});
    const rows = ref<Record<string, Array<[string, JsonValue]>>>({});
    const editingCell = ref<string | null>(null);
    // shallowRef: JsonValue is recursive, and ref<>'s deep UnwrapRef on it makes TS bail
    // out with TS2589. Nothing mutates this value in place anyway, it is replaced wholesale.
    const originalValue = shallowRef<JsonValue>(null);
    const objectAddingProperty = ref(false);
    const objectProperty = ref<string | null>(null);

    const columns = [
      {
        name: 'property',
        align: 'left',
        field: (row) => row[0],
        sortable: true,
      },
      {
        name: 'value',
        align: 'left',
        field: (row) => row[1],
        sortable: true,
      },
    ];

    const initializeData = () => {
      objectNames.value = {};
      rows.value = {};
      for (let k in props.objects) {
        if (Object.prototype.hasOwnProperty.call(props.objects, k)) {
          let obj = props.objects[k];
          objectNames.value[k] = k;
          rows.value[k] = [];
          for (let p in obj) {
            if (Object.prototype.hasOwnProperty.call(obj, p)) {
              let v = obj[p];
              rows.value[k].push([p, v]);
            }
          }
        }
      }
    };

    initializeData();

    watch(
      () => props.objects,
      () => {
        initializeData();
      },
      { deep: true }
    );

    const formatCellValue = (value: JsonValue): string => {
      return typeof value === 'object' ? JSON.stringify(value) : String(value);
    };

    const updateCellValue = (newValue: string, row: [string, JsonValue]) => {
      // console.log('updating', typeof newValue, typeof row[1]);
      try {
        if (typeof row[1] === 'object' || newValue.trim().startsWith('{') || newValue.trim().startsWith('[')) {
          row[1] = JSON.parse(newValue);
        } else {
          if (typeof row[1] === 'number') {
            row[1] = Number(newValue);
          } else if (typeof row[1] === 'boolean') {
            row[1] = newValue === 'true';
          } else {
            row[1] = newValue;
          }
        }
      } catch (e) {
        console.log(e);
        row[1] = newValue;
      }
    };

    let currentEvtElement: HTMLElement | null = null;

    function onRowClick(evt: Event, row: [string, JsonValue], _index: number) {
      if (row[0] === 'init' || row[0] === 'time') {
        // init/time hold a "FUNCTION <n>" placeholder that parser.fun() resolves
        const fun_ref = typeof row[1] === 'string' ? row[1] : '';
        currentEvtElement?.classList.remove('bg-amber-5');
        if (scriptStore.current_function !== fun_ref) {
          scriptStore.set_snippet(projectStore.parser?.fun(fun_ref) || '', true);
          scriptStore.current_function = fun_ref;
          if (evt.target) {
            const parentNode = (evt.target as HTMLElement).parentNode as HTMLElement;
            if (parentNode) {
              parentNode.classList.add('bg-amber-5');
              currentEvtElement = parentNode;
            }
          }
        } else {
          if (evt.target) {
            const parentNode = (evt.target as HTMLElement).parentNode as HTMLElement;
            if (parentNode) {
              parentNode.classList.remove('bg-amber-5');
            }
          }
          scriptStore.set_snippet('', true);
          scriptStore.current_function = '';
        }
      }
    }

    function clearFunSelect() {
      scriptStore.set_snippet('', true);
      currentEvtElement?.classList.remove('bg-amber-5');
      currentEvtElement = null;
      scriptStore.current_function = '';
    }

    const startEditing = (el: HTMLElement, row: [string, JsonValue]) => {
      let prop = row[0];
      let value = row[1];
      editingCell.value = prop;
      originalValue.value = value;
      let par = el.parentNode as HTMLElement;

      // nextTick() is not enough here: quasar creates the edit input on its own schedule,
      // so it can still be absent a tick later. Poll for it, but give up instead of
      // spinning forever when the cell never appears (edit cancelled, row re-rendered).
      let attempts_left = 50; // 50 * 10ms = half a second
      function try_focus() {
        const input = par.querySelector('input');
        if (input) {
          input.focus();
          return true;
        }
        if (--attempts_left > 0) setTimeout(try_focus, 10);
        return false;
      }
      try_focus();
    };

    const onCellChange = (row: [string, JsonValue]) => {
      let prop = row[0];
      let value = row[1];

      if (value === originalValue.value) {
        editingCell.value = null;
        originalValue.value = null;
        return;
      }

      const updatedObjects = { ...props.objects };
      updatedObjects[selected.value][prop] = value;
      emit('update:objects', updatedObjects);
      emit('save-changes');

      editingCell.value = null;
      originalValue.value = null;
    };

    // Add object property (step 1 - open selector)
    function addObjectProperty() {
      scriptStore.request_object_spec_by_user++;
      objectAddingProperty.value = true;
    }

    // Add object property (step 2 - commit selection)
    function addObjectProperty2() {
      if (selected.value && objectProperty.value) {
        const updatedObjects = { ...props.objects };
        const defaultValue =
          scriptStore.object_spec &&
          objectProperty.value in scriptStore.object_spec &&
          'default' in scriptStore.object_spec[objectProperty.value]
            ? scriptStore.object_spec[objectProperty.value].default
            : null;

        if (selected.value in updatedObjects && objectProperty.value) {
          updatedObjects[selected.value][objectProperty.value] = defaultValue;
        }

        // update parent
        emit('update:objects', updatedObjects);

        // add to rows (for local display)
        if (selected.value in rows.value && objectProperty.value) {
          rows.value[selected.value].push([objectProperty.value, defaultValue]);
        }

        objectProperty.value = null;
        objectAddingProperty.value = false;

        emit('save-changes');
      }
    }

    // compute available object properties
    const objectProperties = computed(() => {
      if (!selected.value || !(selected.value in props.objects)) return [];

      let existingProperties = Object.keys(props.objects[selected.value] || {});
      const objectSpec = scriptStore.object_spec || {};
      return Object.keys(objectSpec)
        .filter((prop) => !existingProperties.includes(prop))
        .sort();
    });

    // watch for object spec updates
    watch(
      () => scriptStore.request_object_spec_received,
      () => {
        console.log('Object spec received', scriptStore.object_spec);
      }
    );

    return {
      selected,
      objectNames,
      rows,
      columns,
      editingCell,
      objectAddingProperty,
      objectProperty,
      objectProperties,
      formatCellValue,
      updateCellValue,
      onRowClick,
      clearFunSelect,
      startEditing,
      onCellChange,
      addObjectProperty,
      addObjectProperty2,
    };
  },
});
</script>
