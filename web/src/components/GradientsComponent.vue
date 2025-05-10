<template>
  <div class="row items-center">
    <q-item-label header>Gradients</q-item-label>
    <q-btn flat dense icon="add" @click="addGradient" style="margin-top: -10px" />
  </div>

  <q-item clickable v-ripple v-for="name in gradients_one" :key="name" @click="toggleGradientDetails(name)">
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
  </q-item>

  <q-card v-if="expandedGradient">
    <q-card-section>
      <div class="row q-mb-md">
        <q-btn color="primary" size="sm" icon="add" label="Add Stop" @click="addGradientStop(expandedGradient)" />
      </div>

      <div class="row q-mb-md">
        <q-input
          v-model="expandedGradientNewName"
          type="text"
          dense
          label="Gradient name"
          @change="updateGradientName()"
        />
      </div>

      <div v-for="(stop, index) in editableGradient" :key="index" class="row q-mb-sm items-center">
        <div class="col-2">
          <q-input
            v-model.number="stop.position"
            type="number"
            min="0"
            max="1"
            step="0.05"
            dense
            label="Position"
            @change="updateGradient(expandedGradient)"
          />
        </div>

        <div class="col-1 q-mr-md">
          <div
            class="color-preview"
            :style="{
              backgroundColor: `rgba(${stop.r * 255},${stop.g * 255},${stop.b * 255},${stop.a})`,
              border: '1px solid #ccc',
            }"
          ></div>
        </div>

        <div class="col-2">
          <q-input
            v-model.number="stop.r"
            type="number"
            min="0"
            max="1"
            step="0.1"
            dense
            label="R"
            @change="updateGradient(expandedGradient)"
          />
        </div>

        <div class="col-2">
          <q-input
            v-model.number="stop.g"
            type="number"
            min="0"
            max="1"
            step="0.1"
            dense
            label="G"
            @change="updateGradient(expandedGradient)"
          />
        </div>

        <div class="col-2">
          <q-input
            v-model.number="stop.b"
            type="number"
            min="0"
            max="1"
            step="0.1"
            dense
            label="B"
            @change="updateGradient(expandedGradient)"
          />
        </div>

        <div class="col-1">
          <q-input
            v-model.number="stop.a"
            type="number"
            min="0"
            max="1"
            step="0.1"
            dense
            label="A"
            @change="updateGradient(expandedGradient)"
          />
        </div>

        <div class="col-1">
          <q-btn
            flat
            round
            color="negative"
            icon="delete"
            size="sm"
            @click="removeGradientStop(expandedGradient, index)"
          />
        </div>
      </div>
    </q-card-section>
  </q-card>

  <q-item clickable v-ripple v-for="name in gradients_two" :key="name" @click="toggleGradientDetails(name)">
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
  </q-item>
</template>

<script lang="ts">
import { defineComponent, ref, watch, onMounted, computed, nextTick } from 'vue';
import { useScriptStore } from 'stores/script';
import { useProjectStore } from 'stores/project';
import { JsonWithObjectsParser } from 'components/json_parser';

export default defineComponent({
  name: 'GradientsComponent',

  emits: ['save-changes'],

  setup(props, { emit }) {
    const script_store = useScriptStore();
    const project_store = useProjectStore();
    const parsed = project_store.parser?.parsed();

    const rawGradients = parsed && 'gradients' in parsed ? parsed['gradients'] : {};
    const gradients = ref({ ...rawGradients });
    const stringFormatGradients = ref({});
    const expandedGradient = ref(null);
    const expandedGradientNewName = ref('');
    const editableGradient = ref([]);

    // track which gradients are in string format
    for (const [name, gradient] of Object.entries(rawGradients)) {
      if (typeof gradient === 'string') {
        stringFormatGradients.value[name] = true;
      }
    }

    // ----------------------
    // Gradient format utilities
    // ----------------------

    function isStringGradient(gradient) {
      return typeof gradient === 'string';
    }

    // parse a hex color string into RGB components
    function parseHexColor(hexColor) {
      hexColor = hexColor.replace('#', '');
      const r = parseInt(hexColor.substring(0, 2), 16) / 255;
      const g = parseInt(hexColor.substring(2, 4), 16) / 255;
      const b = parseInt(hexColor.substring(4, 6), 16) / 255;
      return { r, g, b };
    }

    // convert a hex color string with optional alpha into gradient array format
    function parseStringGradient(gradientStr) {
      if (!gradientStr) {
        return [
          { position: 0, r: 1, g: 1, b: 1, a: 1 },
          { position: 0.9, r: 1, g: 1, b: 1, a: 1 },
          { position: 1, r: 1, g: 1, b: 1, a: 0 },
        ];
      }

      // check if there's an alpha value specified with @
      let alpha = 0.9; // default fade position
      let hexColor = gradientStr;

      if (gradientStr.includes('@')) {
        const parts = gradientStr.split('@');
        hexColor = parts[0];
        alpha = parseFloat(parts[1]);
      }

      const { r, g, b } = parseHexColor(hexColor);

      // convert to array format with 3 positions as per spec
      return [
        { position: 0, r, g, b, a: 1 },
        { position: alpha, r, g, b, a: 1 },
        { position: 1, r, g, b, a: 0 },
      ];
    }

    // try to convert array gradient back to string format if possible
    function convertToStringFormat(gradientArray) {
      // check if it follows the pattern needed for string format
      if (gradientArray.length === 3) {
        const first = gradientArray[0];
        const middle = gradientArray[1];
        const last = gradientArray[2];

        // check if it follows the pattern of 0, alpha, 1 positions
        if (first.position === 0 && last.position === 1 && last.a === 0 && first.a === 1 && middle.a === 1) {
          // check if all stops have the same color
          if (
            first.r === middle.r &&
            first.g === middle.g &&
            first.b === middle.b &&
            middle.r === last.r &&
            middle.g === last.g &&
            middle.b === last.b
          ) {
            // convert RGB to hex
            const hexR = Math.round(first.r * 255)
              .toString(16)
              .padStart(2, '0');
            const hexG = Math.round(first.g * 255)
              .toString(16)
              .padStart(2, '0');
            const hexB = Math.round(first.b * 255)
              .toString(16)
              .padStart(2, '0');
            const hex = `#${hexR}${hexG}${hexB}`;

            // if middle position is 0.9 (the default), return just the hex
            if (Math.abs(middle.position - 0.9) < 0.001) {
              return hex;
            } else {
              return `${hex}@${middle.position}`;
            }
          }
        }
      }
      return null;
    }

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

      // update script with current gradients if needed
      const currentScript = project_store.parser.parsed();
      if (currentScript && (!currentScript.gradients || Object.keys(currentScript.gradients).length === 0)) {
        currentScript.gradients = { ...gradients.value };
        //script_store.script = JSON.stringify(currentScript, null, 2);
        script_store.script = project_store.parser.to_string();
      }

      let parsed = project_store.parser.parsed();
      let parsedGradients = parsed && parsed['gradients'] ? parsed['gradients'] : {};

      for (let node of document.querySelectorAll('canvas.gradient_preview')) {
        try {
          let canvas = node as HTMLCanvasElement;
          canvas.width = node.parentNode.offsetWidth || 100;
          let ctx = canvas.getContext('2d');
          if (ctx === null) continue;

          // read gradient name from data-gradient attribute
          let gradient_name = canvas.getAttribute('data-gradient');
          if (!gradient_name) continue;

          let gradient = parsedGradients[gradient_name];
          if (!gradient) continue;

          let gradient_definition;

          // handle both string and array formats for the preview
          if (typeof gradient === 'string') {
            gradient_definition = JSON.stringify(parseStringGradient(gradient));
          } else {
            gradient_definition = JSON.stringify(gradient);
          }

          drawCheckerboard(ctx, canvas);

          let colors =
            typeof gradient_definition !== 'undefined' && window.Module.get_gradient_colors
              ? window.Module.get_gradient_colors(gradient_definition, canvas.width)
              : undefined;

          if (colors && colors.size) {
            let x = 0;
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
        } catch (e) {
          console.error('Error rendering gradient preview:', e);
        }
      }
    }


    function drawCheckerboard(ctx, canvas) {
      if (!ctx) return;
      const colors = ['#333333', '#000000'];
      const blocksize = 5;

      for (let y = 0; y < canvas.height; y += blocksize) {
        for (let x = 0; x < canvas.width; x += blocksize) {
          const colorIndex = Math.floor(x / blocksize) % 2 === Math.floor(y / blocksize) % 2 ? 0 : 1;
          ctx.fillStyle = colors[colorIndex];
          ctx.fillRect(x, y, blocksize, blocksize);
        }
      }
    }

    // Set up canvas resize events
    window.addEventListener('resize', resizeCanvas, false);
    watch(() => script_store.re_render_editor_sidepane, resizeCanvas);
    watch(() => script_store.value_updated_by_user, resizeCanvas);
    onMounted(() => {
      resizeCanvas();
    });

    // ----------------------
    // Gradient Actions
    // ----------------------

    function addGradient() {
      const newName = 'gradient-' + Math.random();
      gradients.value[newName] = [
        {
          position: 0.0,
          r: 1,
          g: 0,
          b: 0,
          a: 1,
        },
        {
          position: 1.0,
          r: 1,
          g: 0,
          b: 0,
          a: 0,
        },
      ];

      // update the script
      const parsedScript = project_store.parser.parsed();
      if (parsedScript) {
        parsedScript.gradients = { ...gradients.value };
        // script_store.script = JSON.stringify(parsedScript, null, 2);
        script_store.script = project_store.parser.to_string();
      }

      emit('save-changes', gradients.value);

      setTimeout(resizeCanvas, 100);
      setTimeout(resizeCanvas, 500);
      setTimeout(resizeCanvas, 1000);
    }

    function toggleGradientDetails(name) {
      if (expandedGradient.value === name) {
        expandedGradient.value = null;
        editableGradient.value = [];
      } else {
        expandedGradient.value = name;
        expandedGradientNewName.value = name;

        if (isStringGradient(gradients.value[name])) {
          editableGradient.value = parseStringGradient(gradients.value[name]);
        } else {
          // Make a deep copy to avoid direct mutation
          editableGradient.value = JSON.parse(JSON.stringify(gradients.value[name]));
        }
      }

      setTimeout(resizeCanvas, 10);
      setTimeout(resizeCanvas, 100);
      setTimeout(resizeCanvas, 1000);
    }

    function addGradientStop(gradientName) {
      // Add a new stop at the middle position relative to existing stops
      const stops = editableGradient.value;
      const positions = stops.map((stop) => stop.position);
      const minPos = Math.min(...positions);
      const maxPos = Math.max(...positions);
      const newPos = (minPos + maxPos) / 2;

      // Default to red with 50% opacity
      editableGradient.value.push({
        position: newPos,
        r: 1,
        g: 0,
        b: 0,
        a: 0.5,
      });

      // Sort by position
      sortGradientStops();
      updateGradient(gradientName);
    }

    function removeGradientStop(gradientName, index) {
      // Don't allow removing if there are only 2 stops
      if (editableGradient.value.length <= 2) {
        return;
      }

      editableGradient.value.splice(index, 1);
      updateGradient(gradientName);
    }

    function sortGradientStops() {
      editableGradient.value.sort((a, b) => a.position - b.position);
    }

    function updateGradient(gradientName) {
      // Make sure all values are numbers
      editableGradient.value.forEach((stop) => {
        stop.position = parseFloat(stop.position);
        stop.r = parseFloat(stop.r);
        stop.g = parseFloat(stop.g);
        stop.b = parseFloat(stop.b);
        stop.a = parseFloat(stop.a);
      });

      // Sort by position
      sortGradientStops();

      // If this was originally a string gradient, try to convert back
      if (stringFormatGradients.value[gradientName]) {
        const stringVersion = convertToStringFormat(editableGradient.value);
        if (stringVersion !== null) {
          // We can convert back to string format
          gradients.value[gradientName] = stringVersion;
        } else {
          // Can't convert back, use the array format
          gradients.value[gradientName] = JSON.parse(JSON.stringify(editableGradient.value));
          // It's no longer in string format
          stringFormatGradients.value[gradientName] = false;
        }
      } else {
        // It was already in array format
        gradients.value[gradientName] = JSON.parse(JSON.stringify(editableGradient.value));
      }

      // Make sure we have a valid gradient object
      if (!gradients.value[gradientName] && gradientName) {
        gradients.value[gradientName] = [
          { position: 0, r: 1, g: 0, b: 0, a: 1 },
          { position: 1, r: 1, g: 0, b: 0, a: 0 },
        ];
      }

      // Update the parsed script with our gradients to ensure they're saved properly
      // Mark123
      const parsedScript = project_store.parser.parsed();
      if (parsedScript) {
        parsedScript.gradients = { ...gradients.value };
        //script_store.script = JSON.stringify(parsedScript, null, 2);
        // TODO: this is duplicated code
        // parsedScript.gradients = gradients.value;
        script_store.script = project_store.parser.to_string();
      }

      // Trigger a re-render of the gradient preview
      emit('save-changes', gradients.value);

      // Schedule multiple redraws to ensure canvas is updated
      setTimeout(resizeCanvas, 100);
      setTimeout(resizeCanvas, 500);
      setTimeout(resizeCanvas, 1000);
    }

    function updateGradientName() {
      const oldName = expandedGradient.value;
      const newName = expandedGradientNewName.value;

      if (oldName && newName && oldName !== newName) {
        gradients.value[newName] = gradients.value[oldName];
        delete gradients.value[oldName];
        expandedGradient.value = newName;

        // if it was a string format gradient, keep that information
        if (stringFormatGradients.value[oldName]) {
          stringFormatGradients.value[newName] = true;
          delete stringFormatGradients.value[oldName];
        }

        // update the parsed script with our gradients to ensure they're saved properly
        const parsedScript = project_store.parser.parsed();
        if (parsedScript) {
          parsedScript.gradients = { ...gradients.value };
          script_store.script = project_store.parser.to_string();
        }

        emit('save-changes', gradients.value);
        nextTick(() => {
          // TODO: check do we need the setTimeouts in these cases?
          setTimeout(resizeCanvas, 100);
          setTimeout(resizeCanvas, 500);
        });
      }
    }

    // split gradients into two parts for UI display (before and after expanded gradient)
    const gradients_one = computed(() => {
      let keys = [];
      for (let key of Object.keys(gradients.value)) {
        keys.push(key);
        if (key == expandedGradient.value) {
          break;
        }
      }
      return keys;
    });

    const gradients_two = computed(() => {
      let keys = [];
      if (!expandedGradient.value) {
        return [];
      }
      let flag = false;
      for (let key of Object.keys(gradients.value)) {
        if (flag) {
          keys.push(key);
        }
        if (key == expandedGradient.value) {
          flag = true;
        }
      }
      return keys;
    });

    return {
      gradients,
      editableGradient,
      addGradient,
      expandedGradient,
      expandedGradientNewName,
      toggleGradientDetails,
      addGradientStop,
      removeGradientStop,
      updateGradient,
      updateGradientName,
      gradients_one,
      gradients_two,
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
.color-preview {
  width: 24px;
  height: 24px;
  border-radius: 4px;
}
</style>
