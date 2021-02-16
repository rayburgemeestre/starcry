<template>
  <div>
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
            <b-menu-item icon="information-outline" label="Files" :active="menu == 'files'" v-on:click="menu = menu == 'files' ? '' : 'files'"></b-menu-item>
            <b-menu-item icon="cash-multiple" label="Script" :active="menu == 'script'" v-on:click="menu = menu == 'script' ? '' : 'script'"></b-menu-item>
            <b-menu-item icon="table" label="Objects" :active="menu == 'objects'" v-on:click="menu = menu == 'objects' ? '' : 'objects'"></b-menu-item>
          </b-menu-list>
          <!--
          <b-menu-list label="Actions">
              <b-menu-item label="Logout"></b-menu-item>
          </b-menu-list>
          -->
        </b-menu>
      </div>
      <div v-if="menu == 'files'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <scripts-component width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu == 'script'" class="column" style="background-color: #c0c0c0; width: 38%;">
        <editor-component v-model="cpp_code" name="js" language="javascript" width="100%" height="100vh - 60px"/>
      </div>
      <div v-if="menu == 'objects'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
        <objects-component v-model="objects" width="100%" height="100vh - 60px"/>
      </div>
      <div class="column" style="background-color: black; max-height: calc(100vh - 120px);">
        <div style="position: relative; z-index: 2;">
          <canvas id="canvas2"
                  style="position: absolute; width: 100%; max-height: calc(100vh - 120px); left: 0; top: 0; z-index: 1; pointer-events: none /* saved my day */;"></canvas>
          <canvas id="canvas" v-on:mousemove="on_mouse_move" v-on:wheel="on_wheel"
                  style="position: absolute; width: 100%; max-height: calc(100vh - 120px); left: 0; top: 0; z-index: 0;"></canvas>
        </div>
      </div>
      <div class="column is-narrow">
        <form>
          <fieldset class="uk-fieldset">
            <div class="uk-margin uk-grid-small uk-child-width-auto uk-grid">
              <label><input class="uk-checkbox" type="radio" checked v-model="render_mode" value="server"> Server-side</label>
              <label><input class="uk-checkbox" type="radio" v-model="render_mode" value="client">  Client-side</label>
            </div>
          </fieldset>
        </form>

        Current render mode: {{ render_mode }}<br/>

        <view-point-component v-bind:value="scale" v-bind:x="view_x"  v-bind:y="view_y" />

        <h2>{{ websock_status }}</h2>
        <h2>{{ websock_status2 }}</h2>
        <h2>{{ websock_status3 }}</h2>
        <h2>{{ websock_status4 }}</h2>

        <button v-shortkey="['ctrl', 's']" @shortkey="menu = menu == 'script' ? '' : 'script'">_</button>
        <button v-shortkey="['ctrl', 'f']" @shortkey="menu = menu == 'files' ? '' : 'files'">_</button>
        <button v-shortkey="['ctrl', 'o']" @shortkey="menu = menu == 'objects' ? '' : 'objects'">_</button>
        <button v-shortkey="[',']" @shortkey="prev_frame()">_</button>
        <button v-shortkey="['.']" @shortkey="next_frame()">_</button>
        <button v-shortkey="['shift', 'o']" @shortkey="get_objects()">_</button>
        <button v-shortkey="['r']" @shortkey="open(filename)">_</button>
        <button v-shortkey="['/']" @shortkey="set_frame(current_frame)">_</button>

        <hr>
        <stats-component />
      </div>
    </div>
    <div class="columns" style="margin: 0px 20px">
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
import StarcryAPI from './util/StarcryAPI'

export default {
  data() {
    return {
      cpp_code: 'Hello world',
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
      scale: 1.,
      view_x: 0,
      view_y: 0,
    };
  },
  components: {
    EditorComponent,
    ScriptsComponent,
    ObjectsComponent,
    PlaybackComponent,
    StatsComponent,
    ViewPointComponent,
  },
  methods: {
    open: function(filename) {
      this.$data.filename = filename;
      this.script_endpoint.send("open " + filename);
      this.bitmap_endpoint.send(JSON.stringify({
        'filename': filename,
        'frame': 0,
      }));
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
      if (this.render_mode == 'server') {
        this.bitmap_endpoint.send(JSON.stringify({
          'filename': this.$data.filename,
          'frame': this.$data.current_frame,
        }));
      }
      else if (this.render_mode == 'client') {
        this.shapes_endpoint.send(this.$data.filename + " " + this.$data.current_frame);
      }
    },
    on_mouse_move: function(e) {
      this.$data.view_x = Module.get_mouse_x();
      this.$data.view_y = Module.get_mouse_y();
    },
    on_wheel: function(event) {
      event.preventDefault();
      let delta = event.deltaY / 1000.;
      if (event.deltaY < 0) {
        this.scale += delta * -1.;
      } else {
        this.scale -= delta;
      }
      // Restrict scale
      this.scale = Math.min(Math.max(1., this.scale), 10.);
      console.log(this.scale);
    },
    get_objects: function () {
      this.objects_endpoint.send(this.$data.filename + " " + this.$data.current_frame);
    }
  },
  watch: {
    queued_frames(new_value) {
      this.process_queue();
    }
  },
  mounted: function() {
    this.bitmap_endpoint = new StarcryAPI(
        'bitmap',
        StarcryAPI.binary_type,
        msg => {
          this.$data.websock_status = msg;
        },
        buffer => {
          Module.set_texture(buffer);
          this.$data.rendering--;
          this.process_queue();
        },
        _ => {}
    );
    this.script_endpoint = new StarcryAPI(
        'script',
        StarcryAPI.text_type,
        msg => {
          this.$data.websock_status2 = msg;
        },
        buffer => {
          this.$data.cpp_code = buffer;
        },
        _ => {
          this.script_endpoint.send("open " + this.$data.filename);
        }
    );
    this.shapes_endpoint = new StarcryAPI(
        'shapes',
        StarcryAPI.text_type,
        msg => {
          this.$data.websock_status3 = msg;
        },
        buffer => {
          Module.set_shapes(buffer);
          this.$data.rendering--;
          this.process_queue();
        },
        _ => {
        }
    );
    this.objects_endpoint = new StarcryAPI(
        'objects',
        StarcryAPI.json_type,
        msg => {
          this.$data.websock_status4 = msg;
        },
        buffer => {
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
        }
    );
  }
}

</script>

<style scoped>
html, body {
  height: 100%;
  margin: 0;
}

.toolbar {
  margin: 0 !important;
}
.toolbar, .toolbar > * {
  background-color: #b1eedb;
}

.main-columns {
  min-height: calc(100vh - 60px*2);
  margin: 0;
  background-color: #f3f3f3;
}

canvas {
  cursor: none !important;
}
canvas:active {
  cursor: none !important;
}
</style>
