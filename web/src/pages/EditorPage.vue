<template>
  <div class="q-pa-md">
    <q-list bordered padding>
      <div class="row items-center">
        <q-item-label header>Gradients</q-item-label>
        <q-btn flat dense icon="add" @click="addGradient" style="margin-top: -10px" />
      </div>

      <q-item clickable v-ripple v-for="name in Object.keys(gradients)" :key="name" @click="toggleGradientDetails(name)">
        <q-item-section>
          <q-item-label class="extra-margin">{{ name }}</q-item-label>
          <q-item-label caption>
            <div class="canvas-container">
              <canvas
                class="gradient_preview"
                :data-gradient="name"
                height="27"
                width="100"
                style="background-color: red"
              ></canvas>
            </div>
          </q-item-label>
        </q-item-section>
        
        <!-- Gradient details section that appears when clicked -->
        <q-item v-if="expandedGradient === name">
          <pre>{{ JSON.stringify(gradients[name], null, 2) }}</pre>
        </q-item>
      </q-item>

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
          <q-table
            v-if="selected == key"
            :rows="rows[key]"
            :columns="columns"
            row-key="name"
            @row-click="on_row_click"
            dense
            :filter="filter"
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
        <q-btn color="primary" label="Update" @click="save_changes"></q-btn>
      </q-item>
    </q-list>
  </div>
</template>

<script lang="ts">
import { defineComponent, nextTick, onMounted, ref, watch, computed } from 'vue';
import { JsonWithObjectsParser } from 'components/json_parser';
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
  components: {},
  setup() {
    let script_store = useScriptStore();
    let project_store = useProjectStore();

    let parsed = project_store.parser?.parsed();
    let gradients = ref(parsed && 'gradients' in parsed ? parsed['gradients'] : {});
    let objects = ref(parsed && 'objects' in parsed ? parsed['objects'] : {});
    let video = ref(parsed && 'video' in parsed ? parsed['video'] : {});
    let selected = ref('');

    function resizeCanvas() {
      let module_already_loaded = !!window.Module;
      if (!module_already_loaded || !window.Module.get_gradient_colors) {
        // reschedule
        setTimeout(resizeCanvas, 100);
        return;
      }

      // get as new
      if (!project_store.parser) {
        project_store.parser = new JsonWithObjectsParser(script_store.script);
      }
      let parsed = project_store.parser.parsed();
      let gradients = parsed ? parsed['gradients'] : {};

      for (let node of document.querySelectorAll('canvas.gradient_preview')) {
        let canvas = node as HTMLCanvasElement;
        canvas.width = node.parentNode.offsetWidth;
        let ctx = canvas.getContext('2d');
        if (ctx === null) continue;
        ctx.fillStyle = 'blue';
        // read gradient name from data-gradient attribute
        let gradient_name = canvas.getAttribute('data-gradient');
        if (!gradient_name) continue;

        let gradient_definition = JSON.stringify(gradients[gradient_name]);
        let colors =
          typeof gradient_definition !== 'undefined'
            ? window.Module.get_gradient_colors(gradient_definition, canvas.width)
            : undefined;

        function drawCheckerboard() {
          if (!ctx) return;
          const colors = ['#333333', '#000000'];
          for (let y = 0; y < canvas.height; y++) {
            for (let x = 0; x < canvas.width; x++) {
              // Select the color based on the current pixel's position
              const blocksize = 5;
              const colorIndex = Math.floor(x / blocksize) % 2 === Math.floor(y / blocksize) % 2 ? 0 : 1;
              ctx.fillStyle = colors[colorIndex];
              ctx.fillRect(x, y, 1, 1);
            }
          }
        }

        drawCheckerboard();

        let x = 0;
        if (!colors) return;
        for (let i = 0; i < colors.size(); ) {
          let r = colors.get(i++);
          let g = colors.get(i++);
          let b = colors.get(i++);
          let a = colors.get(i++) / 255;
          ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${a})`;
          ctx.fillRect(x, 0, 1, canvas.height);
          x++;
        }
      }
    }

    // Resize the canvas to fill browser window dynamically
    window.addEventListener('resize', resizeCanvas, false);

    watch(() => script_store.re_render_editor_sidepane, resizeCanvas);
    watch(() => script_store.value_updated_by_user, resizeCanvas);
    onMounted(() => {
      resizeCanvas();
    });

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
    const save_changes = () => {
      let script = 'ERROR: parser is null';
      if (project_store.parser) {
        script = project_store.parser.parsed().video = video.value;
        script = project_store.parser.to_string();
      }
      // for internal
      script_store.set_value(script, true);
      // for debugger
      script_store.set_result(script, true);
      editing_cell.value = null;
      originalValue.value = null;
    };

    function addGradient() {
      gradients.value['test'] = {};
      console.log('Add gradient clicked');
      // Add your gradient logic here
    }

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

    watch(
      () => script_store.request_video_spec_received,
      () => {
        // TODO: implement further
        console.log('Video spec received', script_store.video_spec);
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

    const expandedGradient = ref(null);
    
    function toggleGradientDetails(name) {
      if (expandedGradient.value === name) {
        expandedGradient.value = null; // collapse if already expanded
      } else {
        expandedGradient.value = name; // expand the clicked gradient
      }
      console.log('Gradient details:', JSON.stringify(gradients.value[name]));
    }

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
      addGradient,
      addObject,
      addVideoProperty,
      addVideoProperty2,
      video_adding_property,
      video_properties,
      video_property,
      expandedGradient,
      toggleGradientDetails,
    };
  },
});
</script>

<style scoped>
.canvas-container {
  width: 100%;
}
.canvas-container canvas {
  margin-left: 16px;
  margin-top: 16px;
}
.extra-margin {
  margin-top: 40px;
}
</style>
