<template>
  <div class="top extended" style="height: 100vh;">
    <div class="columns toolbar">
      <b-navbar>
        <template slot="brand">
          <b-navbar-item>
            <img src="/sc.png" alt="STARCRY" style="max-height: 40px;">
          </b-navbar-item>
        </template>
        <template slot="start">
          <b-navbar-dropdown href="#" label="File">
            <b-navbar-item href="#">Open</b-navbar-item>
            <b-navbar-item href="#">Save</b-navbar-item>
            <b-navbar-item href="#">Save as</b-navbar-item>
            <b-navbar-item href="#">Exit</b-navbar-item>
          </b-navbar-dropdown>
          <b-navbar-dropdown href="#" label="Render">
            <b-navbar-item href="#">Frame</b-navbar-item>
            <b-navbar-item href="#">Video</b-navbar-item>
          </b-navbar-dropdown>
          <b-navbar-dropdown label="Help">
            <b-navbar-item href="#">
              About
            </b-navbar-item>
          </b-navbar-dropdown>
        </template>
      </b-navbar>
      <div style="width: 200px; padding-top: 18px">
        <b-progress :value="connected_percentage" size="is-medium" show-value>
          <span v-if="connected" style="color: white;">OK</span>
          <span v-if="!connected" style="color: red;">CONNECTING</span>
        </b-progress>
      </div>
    </div>
    <div class="columns main-columns">
      <div class="column is-narrow">
        <b-menu>
          <b-menu-list label="Menu">
            <b-menu-item icon="information-outline" label="Files" :active="menu === 'files'" v-on:click="menu = menu === 'files' ? '' : 'files'"></b-menu-item>
            <b-menu-item icon="cash-multiple" label="Script" :active="menu === 'script'" v-on:click="menu = menu === 'script' ? '' : 'script'"></b-menu-item>
            <b-menu-item icon="table" label="Objects" :active="menu === 'objects'" v-on:click="menu = menu === 'objects' ? '' : 'objects'"></b-menu-item>
            <b-menu-item icon="table" label="Help" :active="menu === 'help'" v-on:click="menu = menu === 'help' ? '' : 'help'"></b-menu-item>
            <b-menu-item icon="table" label="Debug" :active="menu === 'debug'" v-on:click="menu = menu === 'debug' ? '' : 'debug'"></b-menu-item>
          </b-menu-list>
          <!--
          <b-menu-list label="Actions">
              <b-menu-item label="Logout"></b-menu-item>
          </b-menu-list>
          -->
        </b-menu>

        <hr>
        <stats-component />

        <br/>
        TRANSIT: {{ rendering }}<br/>
        FRAME: {{ current_frame }}<br/>
        <b-input type="number" v-model="current_frame"></b-input>
        LAST FRAME: {{ max_frames }}<br/>
        NUM CHUNKS: {{ num_chunks }}<br/>
        <b-input type="number" v-model="num_chunks"></b-input>

      </div>
      <div v-if="menu === 'files'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <scripts-component width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu === 'script'" class="column" style="background-color: #c0c0c0; width: 38%;">
        <editor-component v-model="input_source" name="js" language="javascript" width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu === 'objects'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <objects-component v-model="objects" width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu === 'help'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <help-component width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu === 'debug'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <debug-component v-model="debug" width="100%" height="100vh - 60px"/>
      </div>
      <div class="column canvas">
        <div style="position: relative; z-index: 2; /* background-color: #880000; */ height: 100%;">
          <canvas id="canvas2"
                  style="z-index: 1; pointer-events: none /* saved my day */;"></canvas>
          <canvas id="canvas" v-on:mousemove="on_mouse_move" v-on:mousedown="on_mouse_down" v-on:wheel="on_wheel"
                  style="z-index: 0;"></canvas>
        </div>
      </div>
      <div class="column is-narrow" style="overflow: auto; overflow-x: hidden; height: calc(100vh - 120px);">
        <form>
          <fieldset class="uk-fieldset">
            <div class="uk-margin uk-grid-small uk-child-width-auto uk-grid">
              <label><input class="uk-checkbox" type="radio" checked v-model="render_mode" value="server"> Server-side</label>
              <label><input class="uk-checkbox" type="radio" v-model="render_mode" value="client">  Client-side</label>
            </div>
          </fieldset>
        </form>

        Current render mode: {{ render_mode }}<br/>

        <view-point-component ref="vpc" v-bind:viewpoint_settings="viewpoint_settings" v-bind:video="video" v-bind:preview_settings="preview" />
        <div style="display: none;">
          <button v-shortkey="['ctrl', 's']" @shortkey="menu = menu === 'script' ? '' : 'script'">_</button>
          <button v-shortkey="['ctrl', 'f']" @shortkey="menu = menu === 'files' ? '' : 'files'">_</button>
          <button v-shortkey="['ctrl', 'o']" @shortkey="menu = menu === 'objects' ? '' : 'objects'">_</button>
          <button v-shortkey="['ctrl', 'u']" @shortkey="menu = menu === 'debug' ? '' : 'debug'">_</button>
          <button v-shortkey="['ctrl', 'h']" @shortkey="menu = menu === 'help' ? '' : 'help'">_</button>
          <button v-shortkey="[',']" @shortkey="prev_frame()">_</button>
          <button v-shortkey="['.']" @shortkey="next_frame()">_</button>
          <button v-shortkey="['shift', 'o']" @shortkey="get_objects()">_</button>
          <button v-shortkey="['r']" @shortkey="open(filename)">_</button>
          <button v-shortkey="['m']" @shortkey="toggle_pointer()">_</button>
          <button v-shortkey="['/']" @shortkey="set_frame(current_frame)">_</button>
          <button v-shortkey="['t']" @shortkey="toggle_menus()">_</button>
        </div>
      </div>
    </div>
    <div class="columns bottom">
      <playback-component v-bind:value="current_frame" v-bind:max_frames="max_frames" />
    </div>
  </div>
</template>

<script>
import EditorComponent from './components/EditorComponent.vue'
import ScriptsComponent from './components/ScriptsComponent.vue'
import ObjectsComponent from './components/ObjectsComponent.vue'
import HelpComponent from './components/HelpComponent.vue'
import PlaybackComponent from './components/PlaybackComponent.vue'
import StatsComponent from './components/StatsComponent.vue'
import ViewPointComponent from './components/ViewPointComponent.vue'
import DebugComponent from './components/DebugComponent.vue'
import StarcryAPI from './util/StarcryAPI'
import JsonWithObjectsParser from './util/JsonWithObjectsParser'

let time1 = null;
let previous_w = 1920;
let previous_h = 1080;

export default {
  data() {
    return {
      input_source: 'Hello world',
      connected_bitmap: false,
      connected_shapes: false,
      connected_script: false,
      connected_objects: false,
      connected_viewpoint: false,
      menu: '',
      filename: '',
      current_frame : 0,
      num_chunks : 16,
      max_frames : 250,
      rendering: 0,
      max_queued: 10,
      _play: false,
      render_mode: 'server',
      objects: [],
      debug: [],
      viewpoint_settings: {
        scale: 1.,
        view_x: 0,
        view_y: 0,
        canvas_w: 1920,
        canvas_h: 1080,
        offset_x: 0,
        offset_y: 0,
      },
      video: {},
      preview: {},
    };
  },
  computed: {
    connected: function () {
      let b = this.connected_bitmap && this.connected_objects && this.connected_shapes && this.connected_script && this.connected_viewpoint;
      if (b) this.toggle_menus();
      return b;
    },
    connected_percentage: function() {
      let counter = 0;
      if (this.connected_bitmap) counter++;
      if (this.connected_objects) counter++;
      if (this.connected_shapes) counter++;
      if (this.connected_script) counter++;
      if (this.connected_viewpoint) counter++;
      return (counter / 5) * 100;
    }
  },
  components: {
    EditorComponent,
    ScriptsComponent,
    ObjectsComponent,
    HelpComponent,
    PlaybackComponent,
    StatsComponent,
    ViewPointComponent,
    DebugComponent,
  },
  methods: {
    open: function(filename) {
      this.$data.filename = filename;
      this.log('DEBUG', 'script', 'send', "open " + filename);
      this.script_endpoint.send("open " + filename);
      this.log('DEBUG', 'bitmap', 'send', JSON.stringify({
        'filename': filename,
        'frame': 0,
      }));
      this.bitmap_endpoint.send(JSON.stringify({
        'filename': filename,
        'frame': 0,
        'num_chunks': this.$data.num_chunks,
      }));
    },
    toggle_pointer: function() {
      Module.toggle_pointer();
    },
    toggle_menus: function() {
      let top = document.querySelector('.top');
      if (top.classList.contains('extended')) {
        top.classList.remove('extended');
      } else {
        top.classList.add('extended');
      }
    },
    process_queue: function () {
      this._schedule_frames();
    },
    // playback
    play: function () {
      this.$data._play = true;
      this._schedule_frames();
    },
    stop: function () {
      this.$data._play = false;
    },
    prev_frame: function () {
      this.$data.current_frame--;
    },
    next_frame: function () {
      this.$data.current_frame++;
    },
    set_frame: function (frame) {
      if (frame) {
        this.$data.current_frame = frame;
      }
      if (!this.$data._play) {
        this._schedule_frame();
      }
    },
    reset_labels_canvas() {
      var canvas = document.getElementById("canvas2");
      var ctx = canvas.getContext("2d");
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    },
    set_offset: function (x, y) {
      this.$data.viewpoint_settings.offset_x = x;
      this.$data.viewpoint_settings.offset_y = y;
    },
    _schedule_frames: function () {
      if (!this.$data._play) return;
      while (this.$data.rendering < this.$data.max_queued) {
        this._schedule_frame();
        this.$data.current_frame++;
      }
    },
    _schedule_frame: function () {
      this.$data.rendering++;
      if (this.render_mode === 'server') {
        this.log('DEBUG', 'bitmap', 'send', JSON.stringify({
          'filename': this.$data.filename,
          'frame': this.$data.current_frame,
        }));
        this.bitmap_endpoint.send(JSON.stringify({
          'filename': this.$data.filename,
          'frame': this.$data.current_frame,
          'num_chunks': this.$data.num_chunks,
        }));
      }
      else if (this.render_mode === 'client') {
        this.log('DEBUG', 'shapes', 'send', this.$data.filename + " " + this.$data.current_frame);
        this.shapes_endpoint.send(this.$data.filename + " " + this.$data.current_frame);
      }
    },
    on_mouse_move: function(e) {
      this.$data.viewpoint_settings.view_x = Module.get_mouse_x();
      this.$data.viewpoint_settings.view_y = Module.get_mouse_y();
    },
    on_mouse_down: function(e) {
      if (e.ctrlKey) {
        this.$refs.vpc.select();
      } else {
        this.render_objects();
      }
    },
    on_wheel: function(event) {
      event.preventDefault();
      let delta = event.deltaY / 1000.;
      if (event.deltaY < 0) {
        this.viewpoint_settings.scale += delta * -1.;
      } else {
        this.viewpoint_settings.scale -= delta;
      }
      // Restrict scale
      // TODO: this 100 needs to come from some constant, or better yet, some actual setting somewhere..
      this.viewpoint_settings.scale = Math.min(Math.max(0., this.viewpoint_settings.scale), 100.);
      console.log(this.viewpoint_settings.scale);
    },
    get_objects: function () {
      this.log('DEBUG', 'objects', 'send', this.$data.filename + " " + this.$data.current_frame);
      this.$refs.vpc.enable_labels();
      this.objects_endpoint.send(this.$data.filename + " " + this.$data.current_frame);
    },
    log: function(level, api, message, data) {
      const ts = new Date().toISOString();
      this.$data.debug.push({
        'timestamp': ts,
        'level': level,
        'api': api,
        'message': message,
        'data': data,
      });
    },
    update_size: function () {
      let cvs = document.getElementById('canvas').parentNode.parentNode;
      let w = cvs.clientWidth;
      w -= w % 2;
      let h = cvs.clientHeight;
      h -= h % 2;
      if (Math.abs(previous_w - w) < 2 && Math.abs(previous_h - h) < 2) {
        return;
      }
      if (time1) clearTimeout(time1);
      time1 = setTimeout(this.update_size, 1000);
      Module.stop();
      previous_w = w;
      previous_h = h;
      // start might throw
      try {
        this.$data.viewpoint_settings.canvas_w = w;
        this.$data.viewpoint_settings.canvas_h = h;
        Module.start(w, h, w, h);
      } catch (e) {
        console.log(e);
      }
      if (Module.last_buffer) {
        setTimeout(function() { Module.set_texture(Module.last_buffer); }, 10);
      }
    },
    render_objects: function () {
      let canvas1 = document.getElementById("canvas");
      let canvas = document.getElementById("canvas2");
      [canvas.width, canvas.height] = [canvas1.width, canvas1.height];
      let ctx = canvas.getContext("2d");
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.font = "15px Monaco";
      ctx.fillStyle = "white";
      ctx.strokeStyle = 'white';
      let canvas_w = Module.get_canvas_w();
      let canvas_h = Module.get_canvas_h();
      let scale = this.viewpoint_settings.scale;
      function squared_dist(num, num2) {
        return (num - num2) * (num - num2);
      }
      function get_distance(x, y, x2, y2) {
        return Math.sqrt(squared_dist(x, x2) + squared_dist(y, y2));
      }
      for (let obj of this.$data.objects) {
        let center_x = this.viewpoint_settings.offset_x;
        let center_y = this.viewpoint_settings.offset_y;
        let offset_x = 0;
        let offset_y = 0;
        let x = ((obj.x - center_x) * scale) - offset_x + canvas_w / 2;
        let y = ((obj.y - center_y) * scale) - offset_y + canvas_h / 2;
        let view_x = (((this.viewpoint_settings.view_x - canvas_w / 2) )/ scale)+ center_x ;
        let view_y = (((this.viewpoint_settings.view_y - canvas_h / 2) )/ scale)+ center_y ;
        if (get_distance(obj.x, obj.y, view_x, view_y) < 10 / scale) {
          ctx.fillStyle = "red";
        } else {
          ctx.fillStyle = "white";
        }
        ctx.fillRect(x - 5, y - 5, 10, 10);
        ctx.fillText(obj.label, x, y);
        ctx.fillText(obj["#"], x, y + 20);
        ctx.fillText(obj.id, x, y + 40);
        ctx.fillText(obj.unique_id, x, y + 60);
        ctx.fillText(obj.random_hash, x, y + 80);
      }
    }
  },
  watch: {
    current_frame(new_value) {
      this.$data.current_frame = parseInt(new_value);
    },
    num_chunks(new_value) {
      this.$data.num_chunks = parseInt(new_value);
    },
    menu(new_value) {
      setTimeout(this.update_size, 100);
    },
    queued_frames(new_value) {
      this.process_queue();
    }
  },
  mounted: function() {
    Module = {
      canvas: (function() { return document.getElementById('canvas'); })(),
      onRuntimeInitialized: function() {
        console.log("Setting Initial Full HD dimensions");
        Module.start(1920, 1080, 1920, 1080);
      }
    };
    window.addEventListener('resize', this.update_size);
    time1 = setTimeout(this.update_size, 1000);
    var s = document.createElement('script');
    s.setAttribute("src", "client.js");
    document.body.appendChild(s);
    //--------------

    this.bitmap_endpoint = new StarcryAPI(
        'bitmap',
        StarcryAPI.binary_type,
        msg => {
          // this.$data.websock_status = msg;
        },
        buffer => {
          this.log('DEBUG', 'bitmap', 'received texture', 'num bytes: ' + buffer.byteLength);
          Module.last_buffer = buffer;
          Module.set_texture(buffer);
          this.$data.rendering--;
          this.process_queue();
        },
        _ => {
          this.log('DEBUG', 'bitmap', 'websocket connected', '');
          this.$data.connected_bitmap = true;
        },
        _ => {
          this.log('DEBUG', 'bitmap', 'websocket disconnected', '');
          this.$data.connected_bitmap = false;
        }
    );

    this.script_endpoint = new StarcryAPI(
        'script',
        StarcryAPI.text_type,
        msg => {
          // this.$data.websock_status2 = msg;
        },
        buffer => {
          if (buffer[0] === '1') {
            this.$data.filename = buffer.slice(1);
            this.$data.connected_script = true;

            this.log('DEBUG', 'script', 'send', 'open ' + this.$data.filename);
            this.script_endpoint.send("open " + this.$data.filename);
          }
          else if (buffer[0] === '2') {
            buffer = buffer.slice(1);
            this.log('DEBUG', 'script', 'received buffer', 'buffer size: ' + buffer.length);
            let p = new JsonWithObjectsParser(buffer.substr(buffer.indexOf('{')));
            this.$data.input_source = buffer;
            this.$data.video = p.parsed()['video'] || {};
            this.$data.preview = p.parsed()['preview'] || {};
            this.$data.viewpoint_settings.scale = this.$data.video['scale'];
            let total_duration = 0;
            for (let scene of p.parsed()['scenes']) {
              if (scene.duration) {
                total_duration += scene.duration;
              }
            }
            if (!total_duration)
              total_duration = this.$data.video['duration'];
            this.$data.max_frames = Math.floor(total_duration * this.$data.video['fps']);
          }
        },
        _ => {
          this.log('DEBUG', 'script', 'websocket connected', '');
        },
        _ => {
          this.log('DEBUG', 'script', 'websocket disconnected', '');
          this.$data.connected_script = false;
        }
    );
    this.shapes_endpoint = new StarcryAPI(
        'shapes',
        StarcryAPI.text_type,
        msg => {
          // this.$data.websock_status3 = msg;
        },
        buffer => {
          this.log('DEBUG', 'shapes', 'received buffer', 'buffer size: ' + buffer.length);
          Module.set_shapes(buffer);
          this.$data.rendering--;
          this.process_queue();
        },
        _ => {
          this.$data.connected_shapes = true;
          this.log('DEBUG', 'shapes', 'websocket connected', '');
        },
        _ => {
          this.$data.connected_shapes = false;
          this.log('DEBUG', 'shapes', 'websocket disconnected', '');
        },
    );

    this.objects_endpoint = new StarcryAPI(
        'objects',
        StarcryAPI.json_type,
        msg => {
          // this.$data.websock_status4 = msg;
        },
        buffer => {
          this.log('DEBUG', 'objects', 'received objects', 'objects size: ' + buffer.length);
          this.$data.objects = buffer;
          this.render_objects();
        },
        _ => {
          this.log('DEBUG', 'objects', 'websocket connected', '');
          this.$data.connected_objects = true;
        },
        _ => {
          this.log('DEBUG', 'objects', 'websocket disconnected', '');
          this.$data.connected_objects = false;
        },
    );
  }
}

</script>

<style>
html, body, body > div {
  overflow: hidden;
}
* {
  font-family: Consolas,monaco,monospace;
  font-size: 95%;
}

.toolbar {
  margin: 0 !important;
  height: 60px;
}
.toolbar, .toolbar > * {
  background-color: #333333;
  color: white;
}
.columns.bottom {
  background-color: #333333;
  color: white;
  height: 60px;
  /* some tweaking to get slider and buttons neatly aligned */
  padding: 8px 30px;
}
.main-columns {
  min-height: calc(100vh - 60px*2);
  margin: 0;
  background-color: #333333;
  color: white;
}
.extended .main-columns {
  min-height: calc(100vh - 60px * 1);
}
.column.is-narrow {
    width: 200px;
}
canvas {
  cursor: none !important;
}
canvas:active {
  cursor: none !important;
}

/* fixes for dark bg */
.menu-list a, .navbar-link {
    color: white;
}
.navbar-item.has-dropdown a.navbar-item:focus, a.navbar-item:focus-within, a.navbar-item:hover, a.navbar-item.is-active, .navbar-link:focus, .navbar-link:focus-within, .navbar-link:hover, .navbar-link.is-active {
    color: black;
}
.navbar-dropdown .navbar-item {
    font-size: 90%;
}

/* canvas */
.column.canvas {
  background-color: black;
  max-height: calc(100vh - 120px);
  /* uncomment next line for static canvas (for debugging) */
  /* width: 1920px; height: 1080px; */
}
.column.canvas canvas {
  position: absolute;
  width: 100%; max-height: calc(100vh - 120px);
  /* uncomment next line for static canvas (for debugging) */
  /* width: 1920px; height: 1080px; */
  left: 0;
  top: 0;
}
.extended .column.canvas, .extended .column.canvas canvas {
  max-height: calc(100vh - 60px);
}
.extended .column.is-narrow, .extended .columns.bottom {
  display: none;
}

.progress-wrapper {
  height: 24px;
}
</style>
