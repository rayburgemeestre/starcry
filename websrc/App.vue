<template>
  <div style="height: 100vh;">
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
    </div>
    <div class="columns main-columns">
      <div class="column is-narrow">
        <b-menu>
          <b-menu-list label="Menu">
            <b-menu-item icon="information-outline" label="Files" :active="menu === 'files'" v-on:click="menu = menu === 'files' ? '' : 'files'"></b-menu-item>
            <b-menu-item icon="cash-multiple" label="Script" :active="menu === 'script'" v-on:click="menu = menu === 'script' ? '' : 'script'"></b-menu-item>
            <b-menu-item icon="table" label="Objects" :active="menu === 'objects'" v-on:click="menu = menu === 'objects' ? '' : 'objects'"></b-menu-item>
            <b-menu-item icon="table" label="Debug" :active="menu === 'debug'" v-on:click="menu = menu === 'debug' ? '' : 'debug'"></b-menu-item>
          </b-menu-list>
          <!--
          <b-menu-list label="Actions">
              <b-menu-item label="Logout"></b-menu-item>
          </b-menu-list>
          -->
        </b-menu>
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
      <div v-if="menu === 'debug'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <debug-component v-model="debug" width="100%" height="100vh - 60px"/>
      </div>
      <div class="column" style="background-color: black; max-height: calc(100vh - 120px);">
        <div style="position: relative; z-index: 2; background-color: #880000; height: 100%;">
          <canvas id="canvas2"
                  style="position: absolute; width: 100%; max-height: calc(100vh - 120px); left: 0; top: 0; z-index: 1; pointer-events: none /* saved my day */;"></canvas>
          <canvas id="canvas" v-on:mousemove="on_mouse_move" v-on:mousedown="on_mouse_down" v-on:wheel="on_wheel"
                  style="position: absolute; width: 100%; max-height: calc(100vh - 120px); left: 0; top: 0; z-index: 0;"></canvas>
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

        <h2>{{ websock_status }}</h2>
        <h2>{{ websock_status2 }}</h2>
        <h2>{{ websock_status3 }}</h2>
        <h2>{{ websock_status4 }}</h2>

        <div style="display: none;">
          <button v-shortkey="['ctrl', 's']" @shortkey="menu = menu === 'script' ? '' : 'script'">_</button>
          <button v-shortkey="['ctrl', 'f']" @shortkey="menu = menu === 'files' ? '' : 'files'">_</button>
          <button v-shortkey="['ctrl', 'o']" @shortkey="menu = menu === 'objects' ? '' : 'objects'">_</button>
          <button v-shortkey="['ctrl', 'u']" @shortkey="menu = menu === 'debug' ? '' : 'debug'">_</button>
          <button v-shortkey="[',']" @shortkey="prev_frame()">_</button>
          <button v-shortkey="['.']" @shortkey="next_frame()">_</button>
          <button v-shortkey="['shift', 'o']" @shortkey="get_objects()">_</button>
          <button v-shortkey="['r']" @shortkey="open(filename)">_</button>
          <button v-shortkey="['m']" @shortkey="toggle_pointer()">_</button>
          <button v-shortkey="['/']" @shortkey="set_frame(current_frame)">_</button>
        </div>

        <hr>
        <stats-component />
      </div>
    </div>
    <div class="columns bottom">
      <playback-component v-bind:value="current_frame" />
    </div>
  </div>
</template>

<script>
import EditorComponent from './components/EditorComponent.vue'
import ScriptsComponent from './components/ScriptsComponent.vue'
import ObjectsComponent from './components/ObjectsComponent.vue'
import PlaybackComponent from './components/PlaybackComponent.vue'
import StatsComponent from './components/StatsComponent.vue'
import ViewPointComponent from './components/ViewPointComponent.vue'
import DebugComponent from './components/DebugComponent.vue'
import StarcryAPI from './util/StarcryAPI'
import JsonWithObjectsParser from './util/JsonWithObjectsParser'

export default {
  data() {
    return {
      input_source: 'Hello world',
      websock_status: '',
      websock_status2: '',
      websock_status3: '',
      websock_status4: '',
      menu: '',
      filename: 'input/test.js',
      current_frame : 0,
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
      },
      video: {},
      preview: {},
    };
  },
  components: {
    EditorComponent,
    ScriptsComponent,
    ObjectsComponent,
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
      }));
    },
    toggle_pointer: function() {
      Module.toggle_pointer();
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
      this.viewpoint_settings.scale = Math.min(Math.max(1., this.viewpoint_settings.scale), 100.);
      console.log(this.viewpoint_settings.scale);
    },
    get_objects: function () {
      this.log('DEBUG', 'objects', 'send', this.$data.filename + " " + this.$data.current_frame);
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
    }
  },
  watch: {
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
    let time1 = null;
    let previous_w = 1920;
    let previous_h = 1080;
    let update_size = function () {
      let cvs = document.getElementById('canvas').parentNode;
      let w = cvs.clientWidth;
      w -= w % 2;
      let h = cvs.clientHeight;
      h -= h % 2;
      if (Math.abs(previous_w - w) < 2 && Math.abs(previous_h - h) < 2) {
        return;
      }
      if (time1) clearTimeout(time1);
      time1 = setTimeout(update_size, 1000);
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
    }.bind(this);
    window.addEventListener('resize', update_size);
    time1 = setTimeout(update_size, 1000);
    var s = document.createElement('script');
    s.setAttribute("src", "client.js");
    document.body.appendChild(s);
    //--------------

    this.bitmap_endpoint = new StarcryAPI(
        'bitmap',
        StarcryAPI.binary_type,
        msg => {
          this.$data.websock_status = msg;
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
        },
        _ => {
          this.log('DEBUG', 'bitmap', 'websocket disconnected', '');
        }
  );
    this.script_endpoint = new StarcryAPI(
        'script',
        StarcryAPI.text_type,
        msg => {
          this.$data.websock_status2 = msg;
        },
        buffer => {
          this.log('DEBUG', 'script', 'received buffer', 'buffer size: ' + buffer.length);
          let p = new JsonWithObjectsParser(buffer.substr(buffer.indexOf('{')));
          this.$data.input_source = buffer;
          this.$data.video = p.parsed()['video'];
          this.$data.preview = p.parsed()['preview'];
        },
        _ => {
          this.log('DEBUG', 'script', 'websocket connected', '');
          this.log('DEBUG', 'script', 'send', 'open ' + this.$data.filename);
          this.script_endpoint.send("open " + this.$data.filename);
        },
        _ => {
          this.log('DEBUG', 'script', 'websocket disconnected', '');
        }
    );
    this.shapes_endpoint = new StarcryAPI(
        'shapes',
        StarcryAPI.text_type,
        msg => {
          this.$data.websock_status3 = msg;
        },
        buffer => {
          this.log('DEBUG', 'shapes', 'received buffer', 'buffer size: ' + buffer.length);
          Module.set_shapes(buffer);
          this.$data.rendering--;
          this.process_queue();
        },
        _ => {
          this.log('DEBUG', 'shapes', 'websocket connected', '');
        },
        _ => {
          this.log('DEBUG', 'shapes', 'websocket disconnected', '');
        },
    );
    this.objects_endpoint = new StarcryAPI(
        'objects',
        StarcryAPI.json_type,
        msg => {
          this.$data.websock_status4 = msg;
        },
        buffer => {
          this.log('DEBUG', 'objects', 'received objects', 'objects size: ' + buffer.length);
          this.$data.objects = buffer;
          var canvas1 = document.getElementById("canvas");
          var canvas = document.getElementById("canvas2");
          canvas.width = canvas1.width;
          canvas.height = canvas1.height;
          var ctx = canvas.getContext("2d");
          ctx.clearRect(0, 0, canvas.width, canvas.height);
          ctx.font = "12px Monaco";
          ctx.fillStyle = "yellow";
          ctx.strokeStyle = 'yellow';
          var canvas_w = 1920.;
          var canvas_h = 1080.;
          var scale = Module.get_scale();
          for (let obj of buffer) {
            var x = obj.x * scale + canvas_w/2.;
            var y = obj.y * scale + canvas_h/2.
            var offset = 0;
            offset += obj.level;
            ctx.fillText(obj.label, x / canvas_w * canvas.width, y / canvas_h * canvas.height + offset++ * 20);
          }
        },
        _ => {
          this.log('DEBUG', 'objects', 'websocket connected', '');
        },
        _ => {
          this.log('DEBUG', 'objects', 'websocket disconnected', '');
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
</style>
