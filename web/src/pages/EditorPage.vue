<template>
  <div class="q-pa-md">
    <q-list bordered padding>
      <gradients-component @save-changes="save_changes" />

      <q-separator spaced />
      <div class="row items-center">
        <q-item-label header>Objects</q-item-label>
        <q-btn flat dense icon="add" @click="addObject" style="margin-top: -10px" />
      </div>

      <q-item
        tag="label"
        v-ripple
        v-for="(object, key) in objects"
        :key="key"
        @click="
          if (selected != key) clear_fun_select();
          selected = key;
        "
      >
        <q-item-section>
          <q-item-label>{{ key }}</q-item-label>
          <q-item-label caption> {{ object.type }} </q-item-label>
          <q-input v-if="selected == key" v-model="object_names[key]" label="name" />
          <div class="row items-center" v-if="selected == key">
            <q-item-label header>Object Properties</q-item-label>
            <q-btn flat dense icon="add" @click="addObjectProperty" style="margin-top: -10px" />
          </div>
          <q-item v-if="object_adding_property">
            <q-select
              v-model="object_property"
              :options="object_properties"
              label="Object Property"
              class="full-width"
            />
          </q-item>
          <q-item v-if="object_adding_property">
            <q-btn flat dense @click="addObjectProperty2" style="margin-top: -10px"> Add Object Property </q-btn>
          </q-item>
          <q-table
            v-if="selected == key"
            :rows="rows[key]"
            :columns="columns"
            row-key="name"
            @row-click="on_row_click"
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
                    editing_cell === props.row[0]
                  "
                  :model-value="format_cell_value(props.row[1])"
                  @update:model-value="update_cell_value($event, props.row)"
                  dense
                  borderless
                  @blur="on_cell_change(props.row)"
                  @keyup.enter="on_cell_change(props.row)"
                />
                <div
                  v-if="
                    props.col.name === 'value' &&
                    props.row[0] != 'init' &&
                    props.row[0] != 'time' &&
                    editing_cell !== props.row[0]
                  "
                  @click="start_editing($event.target, props.row)"
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
      <q-separator spaced />

      <div class="row items-center">
        <q-item-label header>Video</q-item-label>
        <q-btn flat dense icon="add" @click="addVideoProperty" style="margin-top: -10px" />
      </div>

      <q-item v-if="video_adding_property">
        <q-select v-model="video_property" :options="video_properties" label="Video Property" class="full-width" />
      </q-item>
      <q-item v-if="video_adding_property">
        <q-btn flat dense @click="addVideoProperty2" style="margin-top: -10px"> Add Video Property </q-btn>
      </q-item>

      <q-item clickable v-ripple v-for="name in Object.keys(video)" :key="name" dense>
        <template v-if="name === 'bg_color'">
          <q-input
            v-for="colorKey in ['r', 'g', 'b', 'a']"
            :key="colorKey"
            v-model.number="video[name][colorKey]"
            :label="colorKey"
            type="number"
            min="0"
            max="1"
            step="0.1"
          />
        </template>
        <template v-else-if="typeof video[name] === 'boolean'">
          <q-checkbox v-model="video[name]" :label="name" />
        </template>
        <q-input
          v-else
          v-model="video[name]"
          :label="name"
          :type="typeof video[name] === 'number' ? 'number' : 'text'"
        />
      </q-item>

      <q-separator spaced />

      <q-item>
        <q-btn color="primary" label="Update" @click="save_changes(null)"></q-btn>
      </q-item>
    </q-list>
  </div>
</template>

<script lang="ts">
import { defineComponent, nextTick, onMounted, ref, watch, computed } from 'vue';
import { JsonWithObjectsParser } from 'components/json_parser';
import GradientsComponent from 'components/GradientsComponent.vue';
import { useScriptStore } from 'stores/script';
import { useProjectStore } from 'stores/project';

const format_cell_value = (value) => {
  return typeof value === 'object' ? JSON.stringify(value) : value;
};

const update_cell_value = (newValue, row) => {
  try {
    if (typeof row[1] === 'object' || newValue.trim().startsWith('{') || newValue.trim().startsWith('[')) {
      row[1] = JSON.parse(newValue);
    } else {
      row[1] = newValue;
    }
  } catch (e) {
    row[1] = newValue;
  }
};

export default defineComponent({
  name: 'EditorPage',
  components: {
    GradientsComponent,
  },
  setup() {
    let script_store = useScriptStore();
    let project_store = useProjectStore();

    let parsed = project_store.parser?.parsed();
    let gradients = ref(parsed && 'gradients' in parsed ? parsed['gradients'] : {});
    let objects = ref(parsed && 'objects' in parsed ? parsed['objects'] : {});
    let video = ref(parsed && 'video' in parsed ? parsed['video'] : {});
    let selected = ref('');

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

    let rows = ref({});
    let object_names = ref({});
    for (let k in objects.value) {
      let obj = objects.value[k];
      object_names.value[k] = k;
      rows.value[k] = [];
      for (let p in obj) {
        let v = obj[p];
        rows.value[k].push([p, v]);
      }
    }

    let current_evt_element = false;

    function on_row_click(evt, row, index) {
      if (row[0] === 'init' || row[0] === 'time') {
        if (current_evt_element) {
          current_evt_element.classList.remove('bg-amber-5');
        }
        if (script_store.current_function !== row[1]) {
          script_store.set_snippet(project_store.parser.fun(row[1]), true);
          script_store.current_function = row[1];
          evt.target.parentNode.classList.add('bg-amber-5');
          current_evt_element = evt.target.parentNode;
        } else {
          evt.target.parentNode.classList.remove('bg-amber-5');
          script_store.set_snippet('', true);
          script_store.current_function = '';
        }
      }
    }

    function clear_fun_select() {
      script_store.set_snippet('', true);
      if (current_evt_element) {
        current_evt_element.classList.remove('bg-amber-5');
      }
      current_evt_element = false;
      script_store.current_function = '';
    }

    const editing_cell = ref(null);
    const originalValue = ref(null);

    const start_editing = async (el, row) => {
      let prop = row[0];
      let value = row[1];
      editing_cell.value = prop;
      originalValue.value = value;
      let par = el.parentNode;

      // Wait for the DOM to update
      await nextTick();
      // Focus the input
      setTimeout(function () {
        par.querySelector('input').focus();
      }, 100);
      setTimeout(function () {
        par.querySelector('input').focus();
      }, 1000);
    };

    const on_cell_change = (row) => {
      let prop = row[0];
      let value = row[1];

      // Return early if value hasn't changed
      if (value === originalValue.value) {
        editing_cell.value = null;
        originalValue.value = null;
        return;
      }

      objects.value[selected.value][prop] = value;
      // result is on the debug page
      // script_store.set_result(project_store.parser.to_string(), false);
      save_changes();
    };
    const save_changes = (_gradients) => {
      if (_gradients) {
        gradients.value = _gradients.value;
      }
      let script = 'ERROR: parser is null';
      if (project_store.parser) {
        project_store.parser.parsed().gradients = gradients.value;
        project_store.parser.parsed().video = video.value;
        script = project_store.parser.to_string();
      }
      // for internal
      script_store.set_value(script, true);
      // for debugger
      script_store.set_result(script, true);
      editing_cell.value = null;
      originalValue.value = null;
    };

    function addObject() {
      console.log('Add object clicked');
      // Add your object logic here
    }

    function addVideoProperty() {
      console.log('Add video property clicked');
      script_store.request_video_spec_by_user++;
      video_adding_property.value = true;
    }

    function addVideoProperty2() {
      console.log('Add video property clicked');
      video.value[video_property.value] = script_store.video_spec[video_property.value].default;
      video_property.value = null;
      video_adding_property.value = false;
    }

    function addObjectProperty() {
      console.log('Add object property clicked');
      script_store.request_object_spec_by_user++;
      object_adding_property.value = true;
    }

    function addObjectProperty2() {
      console.log('Add object property clicked');
      if (selected.value && object_property.value) {
        // add to object directly
        objects.value[selected.value][object_property.value] = script_store.object_spec[object_property.value].default;
        // add to rows (further updates will happen through on_cell_change)
        rows.value[selected.value].push([
          object_property.value,
          script_store.object_spec[object_property.value].default,
        ]);

        object_property.value = null;
        object_adding_property.value = false;
      }
    }

    watch(
      () => script_store.request_video_spec_received,
      () => {
        console.log('Video spec received', script_store.video_spec);
      }
    );

    watch(
      () => script_store.request_object_spec_received,
      () => {
        console.log('Object spec received', script_store.object_spec);
      }
    );

    const video_properties = computed(() => {
      let existing_properties = Object.keys(video.value);
      return Object.keys(script_store.video_spec)
        .filter((prop) => !existing_properties.includes(prop))
        .sort();
    });

    const video_property = ref(null);
    let video_adding_property = ref(false);

    const object_properties = computed(() => {
      let existing_properties = Object.keys(objects.value[selected.value] || {});
      return Object.keys(script_store.object_spec)
        .filter((prop) => !existing_properties.includes(prop))
        .sort();
    });

    const object_property = ref(null);
    let object_adding_property = ref(false);

    return {
      parsed,
      gradients,
      objects,
      video,
      columns,
      rows,
      selected,
      object_names,
      on_row_click,
      clear_fun_select,
      editing_cell,
      start_editing,
      on_cell_change,
      save_changes,
      format_cell_value,
      update_cell_value,
      addObject,
      addVideoProperty,
      addVideoProperty2,
      addObjectProperty,
      addObjectProperty2,
      video_adding_property,
      video_properties,
      video_property,
      object_adding_property,
      object_properties,
      object_property,
    };
  },
});
</script>
