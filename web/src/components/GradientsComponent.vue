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
          v-model.number="expandedGradientNewName"
          type="text"
          dense
          label="Gradient name"
          @change="updateGradientName()"
        />
      </div>

      <div v-for="(stop, index) in gradients[expandedGradient]" :key="index" class="row q-mb-sm items-center">
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
import { defineComponent, ref, watch, onMounted, computed } from 'vue';
import { useScriptStore } from 'stores/script';
import { useProjectStore } from 'stores/project';
import { JsonWithObjectsParser } from 'components/json_parser';

export default defineComponent({
  name: 'GradientsComponent',

  emits: ['save-changes'],

  setup(props, { emit }) {
    let script_store = useScriptStore();
    let project_store = useProjectStore();
    let parsed = project_store.parser?.parsed();
    let gradients = ref(parsed && 'gradients' in parsed ? parsed['gradients'] : {});

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

    function addGradient() {
      gradients.value['gradient-' + Math.random()] = [
        {
          position: 0.0,
          r: 1,
          g: 0,
          b: 0,
          a: 1,
        },
      ];
      console.log('Add gradient clicked');
      // Add your gradient logic here
    }
    const expandedGradient = ref(null);
    const expandedGradientNewName = ref(null);

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

    function toggleGradientDetails(name: string) {
      expandedGradient.value = expandedGradient.value === name ? null : name;
      expandedGradientNewName.value = expandedGradient.value;
      setTimeout(resizeCanvas, 10);
      setTimeout(resizeCanvas, 100);
      setTimeout(resizeCanvas, 1000);
    }

    function addGradientStop(gradientName: string) {
      // Add a new stop at the middle position relative to existing stops
      const stops = gradients.value[gradientName];
      const positions = stops.map((stop: { position: number }) => stop.position);
      const minPos = Math.min(...positions);
      const maxPos = Math.max(...positions);
      const newPos = (minPos + maxPos) / 2;

      // Default to red with 50% opacity
      gradients.value[gradientName].push({
        position: newPos,
        r: 1,
        g: 0,
        b: 0,
        a: 0.5,
      });

      // Sort by position
      sortGradientStops(gradientName);
      updateGradient(gradientName);
    }

    function removeGradientStop(gradientName: string, index: number) {
      // Don't allow removing if there are only 2 stops
      if (gradients.value[gradientName].length <= 2) {
        return;
      }

      gradients.value[gradientName].splice(index, 1);
      updateGradient(gradientName);
    }

    function sortGradientStops(gradientName: string) {
      gradients.value[gradientName].sort((a: { position: number }, b: { position: number }) => a.position - b.position);
    }

    function updateGradient(gradientName: string) {
      // Make sure all values are numbers
      gradients.value[gradientName].forEach(
        (stop: { position: number; r: number; g: number; b: number; a: number }) => {
          stop.position = parseFloat(stop.position);
          stop.r = parseFloat(stop.r);
          stop.g = parseFloat(stop.g);
          stop.b = parseFloat(stop.b);
          stop.a = parseFloat(stop.a);
        }
      );

      // Sort by position
      sortGradientStops(gradientName);

      // Trigger a re-render of the gradient preview
      emit('save-changes', gradients);
    }

    function updateGradientName() {
      const oldName = expandedGradient.value;
      const newName = expandedGradientNewName.value;

      if (oldName && newName && oldName !== newName) {
        gradients.value[newName] = gradients.value[oldName];
        delete gradients.value[oldName];
        expandedGradient.value = newName;
        emit('save-changes', gradients);
      }
    }

    return {
      gradients,
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
