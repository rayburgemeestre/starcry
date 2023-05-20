<template>
  <q-layout view="hHh lpR fFf">
    <q-header elevated class="bg-primary text-white" height-hint="98">
      <q-toolbar>
        <q-btn dense flat round icon="menu" @click="toggleLeftDrawer" />
        <q-toolbar-title>
          <img
            src="/sc.png"
            style="position: relative; top: 10px; left: 10px; height: 40px"
            alt="logo"
          />
        </q-toolbar-title>
        <q-btn dense flat round icon="menu" @click="toggleRightDrawer" />
      </q-toolbar>

      <q-tabs align="left">
        <q-route-tab to="/files" label="Files" />
        <q-route-tab to="/script" label="Script" />
        <q-route-tab to="/editor" label="Editor" />
        <q-route-tab to="/objects" label="Objects" />
        <q-route-tab to="/help" label="Help" />
        <q-route-tab to="/debug" label="Debug" />
      </q-tabs>
    </q-header>

    <q-drawer
      show-if-above
      v-model="leftDrawerOpen"
      side="left"
      bordered
      :width="drawerWidth"
    >
      <router-view />
      <div
        v-touch-pan.preserveCursor.prevent.mouse.horizontal="resizeDrawer"
        class="q-drawer__resizer"
      ></div>
    </q-drawer>

    <q-drawer show-if-above v-model="rightDrawerOpen" side="right" bordered>
      <viewpoint-component />
    </q-drawer>

    <q-page-container>
      <div
        style="
          position: relative;
          z-index: 2; /* background-color: #880000; */
          height: 100%;
        "
      >
        <div
          style="
            position: absolute;
            z-index: 1000;
            top: 10px;
            left: 10px;
            color: blue;
          "
        >
          <!-- DEBUG: {{ debug_text }} -->
        </div>
        <canvas
          id="canvas2"
          style="z-index: 1; pointer-events: none; /* saved my day */"
        ></canvas>
        <canvas
          id="canvas"
          ref="canvas_elem"
          v-on:mousemove="on_mouse_move"
          v-on:mousedown="on_mouse_down"
          v-on:wheel="on_wheel"
          v-on:touchstart="on_touchstart"
          v-on:touchmove="on_touchmove"
          v-on:touchend="on_touchend"
          v-pinch="pinchHandler"
          style="z-index: 0"
        ></canvas>
      </div>
    </q-page-container>

    <q-footer elevated class="bg-grey-8 text-white">
      <q-toolbar>
        <q-toolbar-title>
          <timeline-component />
        </q-toolbar-title>
      </q-toolbar>
    </q-footer>
  </q-layout>
</template>

<script lang="ts">
import { defineComponent, ref, watch } from 'vue';
import TimelineComponent from 'components/TimelineComponent.vue';
import { useScriptStore } from 'stores/script';
import ViewpointComponent from 'components/ViewpointComponent.vue';
import { append_script_to_body } from 'components/utils';
import { useViewpointStore } from 'stores/viewpoint';
import { useObjectsStore } from 'stores/objects';
import { usePinch } from '@vueuse/gesture';

const canvas_elem = ref();
let debug_text = ref('');

// this is not the right place, quasar build generates javascript that tries to read client-data before this has executed
// save_client_data(load_client_data());

let my_interval: number = null;

export default defineComponent({
  name: 'MainLayout',

  components: {
    TimelineComponent,
    ViewpointComponent,
  },

  setup() {
    const leftDrawerOpen = ref(false);
    const rightDrawerOpen = ref(false);
    const script = ref('Hello world');

    let script_store = useScriptStore();
    let viewpoint_store = useViewpointStore();

    let initialDrawerWidth;
    let previous_w;
    let previous_h;
    const drawerWidth = ref(300);

    // let holdTimer = null;
    // let startXY = [];

    // Composable usage
    let obj = {
      canvas_elem,
      debug_text,
      script,
      leftDrawerOpen,
      rightDrawerOpen,
      drawer: ref(false),
      drawerWidth,

      // table
      filter: ref(''),
      script_store,

      on_mouse_move: function (e) {
        viewpoint_store.view_x = Module.get_mouse_x();
        viewpoint_store.view_y = Module.get_mouse_y();
      },
      on_touchmove: function (e) {
        viewpoint_store.view_x = Module.get_mouse_x();
        viewpoint_store.view_y = Module.get_mouse_y();
      },
      on_touchstart: function (e) {
        // if (startXY === null) return; // wait till we release the hold
        // startXY = [window.Module.get_mouse_x(), window.Module.get_mouse_y()];
        // if (holdTimer !== null) {
        //   clearTimeout(holdTimer);
        //   holdTimer = null;
        // }
        // holdTimer = setTimeout(function () {
        //   let nowXY = [window.Module.get_mouse_x(), window.Module.get_mouse_y()];
        //   // calculate distance between startXY and nowXY coordinates
        //   let distance = Math.sqrt(
        //     Math.pow(nowXY[0] - startXY[0], 2) + Math.pow(nowXY[1] - startXY[1], 2)
        //   );
        //   debug_text.value = distance;
        //   if (distance < 30) {
        //     // if distance is less than 5px it's a center-here click
        //     viewpoint_store.select();
        //     startXY = null;
        //   }
        //
        // }, 1000);
      },
      on_touchend: function (e) {
        // debug_text.value = "done";
        // // release the hold
        // startXY = [];
        this.render_objects(true);
      },
      on_mouse_down: function (e) {
        if (e.ctrlKey) {
          viewpoint_store.select();
        } else {
          this.render_objects(true);
        }
      },
      render_objects: function (update_selected_objects) {
        let canvas1 = document.getElementById('canvas') as HTMLCanvasElement;
        let canvas = document.getElementById('canvas2') as HTMLCanvasElement;
        [canvas.height, canvas.height] = [canvas1.width, canvas1.height];
        let ctx = canvas.getContext('2d') as CanvasRenderingContext2D;
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.font = '15px Monaco';
        ctx.fillStyle = 'white';
        ctx.strokeStyle = 'white';

        let canvas_w = Module.get_canvas_w();
        let canvas_h = Module.get_canvas_h();
        let texture_w = Module.get_texture_w();
        let texture_h = Module.get_texture_h();
        ctx.fillText(
          'canvas: ' +
            canvas_w +
            ' x ' +
            canvas_h +
            ', texture: ' +
            texture_w +
            ' x ' +
            texture_h,
          20,
          20
        );
        script_store.texture_w = texture_w;
        script_store.texture_h = texture_h;
        console.log(
          'Updated texture dimensions: ' + texture_w + 'x' + texture_h
        );
        script_store.texture_size_updated_by_server++; // make sure our viewpont reacts

        let viewpoint_store = useViewpointStore();
        let scale_ratio = canvas_h / 1080.0;
        let scale = viewpoint_store.scale * scale_ratio;
        function squared_dist(num: number, num2: number) {
          return (num - num2) * (num - num2);
        }
        function get_distance(x: number, y: number, x2: number, y2: number) {
          return Math.sqrt(squared_dist(x, x2) + squared_dist(y, y2));
        }
        let objects_store = useObjectsStore();
        if (update_selected_objects) {
          script_store.clearSelectedObjects();
        }
        for (let obj of objects_store.objects) {
          let center_x = viewpoint_store.offset_x;
          let center_y = viewpoint_store.offset_y;
          let offset_x = 0;
          let offset_y = 0;
          // TODO: refactor the way we send objects once more

          let x = (obj.x - center_x) * scale - offset_x + canvas_w / 2;
          let y = (obj.y - center_y) * scale - offset_y + canvas_h / 2;
          let view_x =
            (viewpoint_store.view_x - canvas_w / 2) / scale + center_x;
          let view_y =
            (viewpoint_store.view_y - canvas_h / 2) / scale + center_y;
          if (
            get_distance(obj.x, obj.y, view_x, view_y) < 10 / scale ||
            script_store.selected.includes(obj.unique_id)
          ) {
            ctx.fillStyle = 'red';
            if (update_selected_objects) {
              script_store.addSelectedObject(obj.unique_id);
            }
          } else {
            ctx.fillStyle = 'white';
          }
          ctx.fillRect(x - 5, y - 5, 10, 10);
          ctx.fillText(obj.label, x, y);
          ctx.fillText('' + obj['#'], x, y + 20);
          ctx.fillText(obj.id, x, y + 40);
          ctx.fillText('' + obj.unique_id, x, y + 60);
          // ctx.fillText(obj.random_hash, x, y + 80);
          // ctx.fillText(obj.time, x, y + 100);
        }
        // draw the actual canvas of the video
        ctx.strokeStyle = 'red';
        ctx.lineWidth = 2;
        let x = canvas_w / 2 - (script_store.video.width / 2) * scale,
          y = canvas_h / 2 - (script_store.video.height / 2) * scale,
          w = script_store.video.width * scale,
          h = script_store.video.height * scale;
        ctx.strokeRect(x, y, w, h);
      },
      on_wheel: function (event) {
        event.preventDefault();
        // eslint-disable-next-line @typescript-eslint/no-loss-of-precision
        let delta = event.deltaY / 1000;
        if (event.deltaY < 0) {
          viewpoint_store.scale += delta * -1;
        } else {
          viewpoint_store.scale -= delta;
        }
        // Restrict scale
        // TODO: this 100 needs to come from some constant, or better yet, some actual setting somewhere..
        // eslint-disable-next-line @typescript-eslint/no-loss-of-precision
        viewpoint_store.scale = Math.min(
          Math.max(0, viewpoint_store.scale),
          100
        );
      },

      handleKeydown(event) {
        if (event.ctrlKey && event.key === '/') {
        } else if (event.ctrlKey && event.key === '.') {
        } else if (event.ctrlKey && event.key === ',') {
        } else if (event.altKey && event.key === 'z') {
        }
      },
    };
    obj.toggleLeftDrawer = function () {
      leftDrawerOpen.value = !leftDrawerOpen.value;
      this.update_size_delayed();
    }.bind(obj);

    obj.toggleRightDrawer = function () {
      rightDrawerOpen.value = !rightDrawerOpen.value;
      this.update_size_delayed();
    }.bind(obj);

    obj.resizeDrawer = function (ev) {
      if (ev.isFirst === true) {
        initialDrawerWidth = drawerWidth.value;
      }
      drawerWidth.value = initialDrawerWidth + ev.offset.x;
      this.update_size_delayed();
    }.bind(obj);

    obj.update_size_delayed = function () {
      if (window.time1) clearTimeout(window.time1);
      window.time1 = setTimeout(this.update_size, 100);
    }.bind(obj);

    obj.update_size = function () {
      let cvs = document
        .getElementById('canvas')
        .parentNode.getBoundingClientRect();
      let header = document.querySelector('.q-header').getBoundingClientRect();
      let footer = document.querySelector('.q-footer').getBoundingClientRect();
      let w = Math.floor(cvs['width']);
      let h = Math.floor(footer['y'] - header['height']);
      document.getElementById('canvas').width = w;
      document.getElementById('canvas').height = h;
      if (Math.abs(previous_w - w) < 2 && Math.abs(previous_h - h) < 2) {
        return;
      }

      script_store.re_render_editor_sidepane++;

      document.getElementById('canvas2').width = w;
      document.getElementById('canvas2').height = h;
      try {
        Module.stop();
      } catch (e) {
        // we'll know soon enough if the thing has crashed...
        // console.log(e);
      }
      previous_w = w;
      previous_h = h;
      // start might throw
      try {
        viewpoint_store.canvas_w = w;
        viewpoint_store.canvas_h = h;
        Module.start(w, h, w, h);
      } catch (e) {
        // we'll know soon enough if the thing has crashed...
        // console.log(e);
      }
      script_store.render_requested_by_user++;
      if (Module.last_buffer) {
        setTimeout(function () {
          Module.set_texture(Module.last_buffer);
        }, 10);
      }
    }.bind(obj);

    const pinchHandler = ({ offset: [d, a], pinching }) => {
      const zoom = d / 256;
      debug_text.value = 1 + zoom;
      viewpoint_store.scale = 1 + zoom;
      // set({ zoom: d, rotateZ: a })
    };

    usePinch(pinchHandler, {
      domTarget: canvas_elem,
      eventOptions: {
        passive: true,
      },
    });

    watch(
      () => script_store.render_completed_by_server,
      (n) => {
        obj.render_objects(false);
      }
    );

    return obj;
  },
  mounted() {
    let viewpoint_store = useViewpointStore();
    let module_already_loaded = !!window.Module;
    if (module_already_loaded) {
      // stop
      try {
        window.Module.stop();
      } catch (e) {}
      // update canvas (element could have been recreated by hot reload dev stuff
      window.Module.canvas = document.getElementById('canvas');
      // start, will update with correct width, height etc., later.
      try {
        window.Module.start(
          viewpoint_store.canvas_w,
          viewpoint_store.canvas_h,
          viewpoint_store.canvas_w,
          viewpoint_store.canvas_h
        );
      } catch (e) {}
      return;
    }

    window.Module = {
      canvas: (function () {
        return document.getElementById('canvas');
      })(),
      onRuntimeInitialized: function () {
        try {
          // might throw
          console.log('Setting Initial Full HD dimensions');
          window.Module.start(
            viewpoint_store.canvas_w,
            viewpoint_store.canvas_h,
            viewpoint_store.canvas_w,
            viewpoint_store.canvas_h
          );
        } catch (e) {
          if (e !== 'unwind') console.error(e);
        }
      },
      print: function (text) {
        console.log('stdout: ' + text);
      },
      printErr: function (text) {
        console.log('stderr: ' + text);
      },
    };

    this.update_size_delayed();

    // schedule function this.update_size_delayed to be called every second
    if (my_interval) {
      clearInterval(my_interval);
    }
    my_interval = setInterval(this.update_size_delayed, 1000);

    // TODO: no idea why this needs some initial delay, the Module object it waits for should already be there
    setTimeout(function () {
      append_script_to_body('client.js');
    }, 100);

    window.addEventListener('keydown', this.handleKeydown);
  },
  beforeUnmount() {
    window.removeEventListener('keydown', this.handleKeydown);
  },
});
</script>

<style>
html,
body {
  overflow: hidden;
}

.q-drawer__resizer {
  position: absolute;
  top: 0;
  bottom: 0;
  right: -2px;
  width: 4px;
  background-color: #990000;
  cursor: ew-resize;
}

.q-drawer__resizer:after {
  content: '';
  position: absolute;
  top: 50%;
  height: 30px;
  left: -5px;
  right: -5px;
  transform: translateY(-50%);
  background-color: inherit;
  border-radius: 4px;
}

canvas {
  position: absolute;
  left: 0;
  top: 0;
  cursor: none !important;
}

canvas:active {
  cursor: none !important;
}
</style>
