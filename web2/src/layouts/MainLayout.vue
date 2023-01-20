<template>
  <q-layout view="hHh lpR fFf">
    <q-header elevated class="bg-primary text-black" height-hint="98">
      <q-toolbar>
        <q-btn dense flat round icon="menu" @click="toggleLeftDrawer" />

        <q-toolbar-title>
          <img
            src="sc.png"
            style="position: relative; top: 10px; left: 10px; height: 40px"
          />
        </q-toolbar-title>

        <q-btn dense flat round icon="menu" @click="toggleRightDrawer" />
      </q-toolbar>

      <q-tabs align="left">
        <q-route-tab to="/files" label="Files" />
        <q-route-tab to="/script" label="Script" />
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
      <!-- drawer content -->
      ATTRIBUTES HERE
    </q-drawer>

    <q-page-container>
      <div
        style="
          position: relative;
          z-index: 2; /* background-color: #880000; */
          height: 100%;
        "
      >
        <canvas
          id="canvas2"
          style="z-index: 1; pointer-events: none /* saved my day */"
        ></canvas>
        <canvas
          id="canvas"
          v-on:mousemove="on_mouse_move"
          v-on:mousedown="on_mouse_down"
          v-on:wheel="on_wheel"
          style="z-index: 0"
        ></canvas>
      </div>
    </q-page-container>

    <q-footer elevated class="bg-grey-8 text-white">
      <q-toolbar>
        <q-toolbar-title>
          <div>TIME NAVIGATION HERE</div>
        </q-toolbar-title>
      </q-toolbar>
    </q-footer>
  </q-layout>
</template>

<script>
import { defineComponent, ref } from 'vue';
import { StarcryAPI } from 'components/api';
import { load_client_data, save_client_data } from 'components/clientstorage';

save_client_data(load_client_data());

export default defineComponent({
  name: 'MainLayout',

  components: {},

  setup() {
    const leftDrawerOpen = ref(false);
    const rightDrawerOpen = ref(false);
    const script = ref('Hello world');
    const script_endpoint = new StarcryAPI(
      'script',
      StarcryAPI.text_type,
      (msg) => {
        console.log(msg);
      },
      (buffer) => {
        console.log(buffer);
      },
      (_) => {
        console.log('connected');
      },
      (_) => {
        console.log('disconnected');
      }
    );
    script_endpoint.connect();

    let initialDrawerWidth;
    const drawerWidth = ref(300);

    return {
      script,
      leftDrawerOpen,
      toggleLeftDrawer() {
        leftDrawerOpen.value = !leftDrawerOpen.value;
        if (window.time1) clearTimeout(window.time1);
        window.time1 = setTimeout(this.update_size, 1000);
      },

      rightDrawerOpen,
      toggleRightDrawer() {
        rightDrawerOpen.value = !rightDrawerOpen.value;
        if (window.time1) clearTimeout(window.time1);
        window.time1 = setTimeout(this.update_size, 1000);
      },

      drawer: ref(false),
      drawerWidth,
      resizeDrawer(ev) {
        if (ev.isFirst === true) {
          initialDrawerWidth = drawerWidth.value;
        }
        drawerWidth.value = initialDrawerWidth + ev.offset.x;
        if (window.time1) clearTimeout(window.time1);
        window.time1 = setTimeout(this.update_size, 1000);
      },

      on_mouse_move: function (e) {
        Module.get_mouse_x(); // force redraw
        // this.$data.viewpoint_settings.view_x = Module.get_mouse_x();
        // this.$data.viewpoint_settings.view_y = Module.get_mouse_y();
      },
      on_mouse_down: function (e) {
        // if (e.ctrlKey) {
        //   this.$refs.vpc.select();
        // } else {
        //   this.render_objects();
        // }
      },
      on_wheel: function (event) {
        // event.preventDefault();
        // let delta = event.deltaY / 1000.;
        // if (event.deltaY < 0) {
        //   this.viewpoint_settings.scale += delta * -1.;
        // } else {
        //   this.viewpoint_settings.scale -= delta;
        // }
        // // Restrict scale
        // // TODO: this 100 needs to come from some constant, or better yet, some actual setting somewhere..
        // this.viewpoint_settings.scale = Math.min(Math.max(0., this.viewpoint_settings.scale), 100.);
        // console.log(this.viewpoint_settings.scale);
      },
      update_size: function () {
        let cvs = document
          .getElementById('canvas')
          .parentNode.getBoundingClientRect();
        let header = document
          .querySelector('.q-header')
          .getBoundingClientRect();
        let footer = document
          .querySelector('.q-footer')
          .getBoundingClientRect();
        let w = Math.floor(cvs['width']) - 10;
        let h = Math.floor(footer['y'] - header['height']) - 10;
        document.getElementById('canvas').width = w;
        document.getElementById('canvas').height = h;
        // if (Math.abs(previous_w - w) < 2 && Math.abs(previous_h - h) < 2) {
        //   return;
        // }
        if (window.time1) clearTimeout(window.time1);
        window.time1 = setTimeout(this.update_size, 1000);
        Module.stop();
        // previous_w = w;
        // previous_h = h;
        // start might throw
        try {
          // this.$data.viewpoint_settings.canvas_w = w;
          // this.$data.viewpoint_settings.canvas_h = h;
          Module.start(w, h, w, h);
        } catch (e) {
          console.log(e);
        }
        if (Module.last_buffer) {
          setTimeout(function () {
            Module.set_texture(Module.last_buffer);
          }, 10);
        }
      },
    };
  },
  mounted() {
    window.Module = {
      canvas: (function () {
        return document.getElementById('canvas');
      })(),
      onRuntimeInitialized: function () {
        console.log('Setting Initial Full HD dimensions');
        //Module.start(1920, 1080, 1920, 1080);
        Module.start(100, 100, 100, 100);
      },
      print: function (text) {
        console.log('stdout: ' + text);
      },
      printErr: function (text) {
        console.log('stderr: ' + text);
      },
    };

    window.addEventListener('resize', this.update_size);
    window.time1 = setTimeout(this.update_size, 1000);

    const s = document.createElement('script');
    s.setAttribute('src', 'client.js');
    document.body.appendChild(s);
  },
});
</script>

<style>
.q-drawer__resizer {
  position: absolute;
  top: 0;
  bottom: 0;
  right: -2px;
  width: 4px;
  background-color: #b9f0de;
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
  /*min-height: calc(500px);*/
  /*width: 100%; max-height: calc(100vh - 120px);*/
  /* uncomment next line for static canvas (for debugging) */
  /* width: 1920px; height: 1080px; */
  left: 0;
  top: 0;
  cursor: none !important;
}
canvas:active {
  cursor: none !important;
}
</style>
