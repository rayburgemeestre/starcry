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
      <!--
      <q-toolbar>
        <q-btn flat round dense icon="draw" />
        <q-toolbar-title>&nbsp;</q-toolbar-title>
      </q-toolbar>
      -->
      <!-- drawer content -->
      <br />
      <q-btn
        :loading="loading"
        color="secondary"
        @click="render_current_frame"
        style="width: 100%"
        >Render</q-btn
      >
      <br />
      <br />
      <q-btn color="secondary" style="width: 50%">Previous</q-btn>
      <q-btn color="secondary" style="width: 50%">Next</q-btn>

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
        <canvas
          id="canvas2"
          style="z-index: 1; pointer-events: none; /* saved my day */"
        ></canvas>
        <canvas
          id="canvas"
          v-on:mousemove="on_mouse_move"
          v-on:mousedown="on_mouse_down"
          v-on:wheel="on_wheel"
          v-on:touchstart="on_touchstart"
          style="z-index: 0"
        ></canvas>
      </div>
    </q-page-container>

    <q-footer elevated class="bg-grey-8 text-white">
      <q-toolbar>
        <q-toolbar-title>
          <timeline-component
            v-bind:value="current_frame"
            v-bind:max_frames="max_frames"
            v-bind:frames_per_scene="frames_per_scene"
          />
        </q-toolbar-title>
      </q-toolbar>
    </q-footer>
  </q-layout>
</template>

<script>
import { defineComponent, ref } from 'vue';
import TimelineComponent from 'components/TimelineComponent.vue';
import { load_client_data, save_client_data } from 'components/clientstorage';
import { StarcryAPI } from 'components/api';
import { useScriptStore } from 'stores/script';
import ViewpointComponent from 'components/ViewpointComponent.vue';
import { append_script_to_body } from 'components/utils';
import { useViewpointStore } from 'stores/viewpoint';

save_client_data(load_client_data());

let bitmap_endpoint = new StarcryAPI(
  'bitmap',
  StarcryAPI.binary_type,
  (msg) => {
    // this.$data.websock_status = msg;
  },
  // eslint-disable-next-line @typescript-eslint/no-empty-function
  (buffer) => {},
  (_) => {
    // this.log('DEBUG', 'bitmap', 'websocket connected', '');
    // this.$data.connected_bitmap = true;
  },
  (_) => {
    // this.log('DEBUG', 'bitmap', 'websocket disconnected', '');
    // this.$data.connected_bitmap = false;
  }
);

let loading = ref(false);

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

    const current_frame = ref(1);
    const max_frames = ref(10);
    const frames_per_scene = ref([5, 5]);

    let obj = {
      loading,
      current_frame,
      max_frames,
      frames_per_scene,
      script,
      leftDrawerOpen,
      rightDrawerOpen,
      drawer: ref(false),
      drawerWidth,

      // table
      filter: ref(''),
      script_store,

      render_current_frame: function () {
        loading.value = true;
        bitmap_endpoint.send(
          JSON.stringify({
            filename: script_store.filename,
            // frame: current_frame.value,
            // random frame between 1 and 150
            frame: script_store.frame,
            // viewpoint_settings: viewpoint_settings,
            num_chunks: 1,
          })
        );
      },

      on_mouse_move: function (e) {
        window.Module.get_mouse_x(); // force redraw
        // this.$data.viewpoint_settings.view_x = Module.get_mouse_x();
        // this.$data.viewpoint_settings.view_y = Module.get_mouse_y();
      },
      on_touchstart: function (e) {
        window.Module.get_mouse_x(); // force redraw
      },
      on_mouse_down: function (e) {
        // if (e.ctrlKey) {
        //   this.$refs.vpc.select();
        // } else {
        //   this.render_objects();
        // }
      },
      on_wheel: function (event) {
        event.preventDefault();
        // eslint-disable-next-line @typescript-eslint/no-loss-of-precision
        let delta = event.deltaY / 1000.;
        if (event.deltaY < 0) {
          viewpoint_store.scale += delta * -1.;
        } else {
          viewpoint_store.scale -= delta;
        }
        // Restrict scale
        // TODO: this 100 needs to come from some constant, or better yet, some actual setting somewhere..
        // eslint-disable-next-line @typescript-eslint/no-loss-of-precision
        viewpoint_store.scale = Math.min(Math.max(0., viewpoint_store.scale), 100.);
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
      if (Module.last_buffer) {
        setTimeout(function () {
          Module.set_texture(Module.last_buffer);
        }, 10);
      }
    }.bind(obj);

    return obj;
  },
  mounted() {
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
        window.Module.start(1920, 1080, 1920, 1080);
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
          Module.start(1920, 1080, 1920, 1080);
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

    // TODO: no idea why this needs some initial delay, the Module object it waits for should already be there
    setTimeout(function () {
      append_script_to_body('client.js');
    }, 100);

    bitmap_endpoint.on_message = (buffer) => {
      window.Module.last_buffer = buffer;
      window.Module.set_texture(buffer);
      // this.$data.rendering--;
      // this.process_queue();
      loading.value = false;
    };
    bitmap_endpoint.connect();
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
