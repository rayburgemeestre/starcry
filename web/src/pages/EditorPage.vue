<template>
  <div class="q-pa-md">
    <q-list bordered padding>
      <q-item-label header>Gradients</q-item-label>

      <q-item clickable v-ripple v-for="name in Object.keys(gradients)" :key="name">
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

      <q-separator spaced />
      <q-item-label header>Objects</q-item-label>

      <q-item tag="label" v-ripple v-for="(object, key) in objects" :key="key" @click="selected = key">
        <q-item-section>
          <q-item-label>{{ key }}</q-item-label>
          <q-item-label caption> {{ object.type }} </q-item-label>
          <q-input v-if="selected == key" v-model="object_names[key]" label="name" />
          <q-table
            v-if="selected == key"
            dense
            :rows="rows[key]"
            :columns="columns"
            row-key="name"
            @row-click="on_row_click"
            :filter="filter"
            hide-header
            hide-pagination
            :rows-per-page-options="[100]"
          >
          </q-table>
        </q-item-section>
      </q-item>
    </q-list>
  </div>
</template>

<script lang="ts">
import { defineComponent, onMounted, ref, watch } from 'vue';
import { JsonWithObjectsParser } from 'components/json_parser';
import { useScriptStore } from 'stores/script';
export default defineComponent({
  name: 'EditorPage',
  components: {},
  setup() {
    let script_store = useScriptStore();
    let p = new JsonWithObjectsParser(script_store.script);
    let parsed = p.parsed();
    let gradients = ref(parsed ? parsed['gradients'] : {});
    let objects = ref(parsed ? parsed['objects'] : {});
    let selected = ref('');

    function resizeCanvas() {
      // get as new
      let p = new JsonWithObjectsParser(script_store.script);
      let parsed = p.parsed();
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
    for (let k in objects) {
      let obj = objects[k];
      object_names.value[k] = k;
      rows.value[k] = [];
      for (let p in obj) {
        let v = obj[p];
        rows.value[k].push([p, '' + v]);
      }
    }

    function on_row_click(evt, row, index) {
      console.log(evt);
      console.log(row);
      console.log(index);
    }

    return {
      parsed,
      gradients,
      objects,
      columns,
      rows,
      selected,
      object_names,
      on_row_click,
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
