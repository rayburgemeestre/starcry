<template>
  <div>
    Scale:
    <b-slider style="width: calc(100%); margin-right: 10px; float: left;" :min="0" :max="10" v-model="scale" ticks></b-slider>

    <br/>

    <label class="checkbox"><input type="checkbox" v-model="auto_render"> <span>auto render</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="raw"> <span>raw</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="preview"> <span>preview</span></label> <br/>
    <br/>

    Scale: {{ scale }} <br/>
    View X: {{ view_x }} <br/>
    View Y: {{ view_y }} <br/>
    View Scale: {{ view_scale }} <br/>
    Offset X: {{ offsetX }} <br/>
    Offset Y: {{ offsetY }} <br/>

    <b-button @click="reset">Reset</b-button>

  </div>
</template>

<script>
import StarcryAPI from '../util/StarcryAPI'
var timer = false;
export default {
  props: {
    value: {
      type: Number,
      required: true
    },
    x: {
      type: Number,
      required: true
    },
    y: {
      type: Number,
      required: true
    },
  },
  data() {
    return {
      scale: 1.,
      previous_scale: 1.,
      view_x: 0,
      view_y: 0,
      view_scale: 1.,
      offsetX: 0.,
      offsetY: 0.,
      auto_render: true,
      raw: false,
      preview: true,
    }
  },
  methods: {
    'update': function() {
      if (timer !== false) {
        clearTimeout(timer);
        timer = false;
      }
      timer = setTimeout(this.scheduled_update.bind(this), 50);
    },
    'scheduled_update': function() {
      this.$data.previous_scale = this.$data.scale;
      this.viewpoint_endpoint.send(JSON.stringify({
        'operation': 'set',
        'scale': this.$data.scale,
        'offset_x': this.$data.offsetX,
        'offset_y': this.$data.offsetY,
        'raw': this.$data.raw,
        'preview': this.$data.preview,
      }));
      if (this.$data.scale === this.$data.previous_scale) return;
      if (this.$data.auto_render) { this.$parent.set_frame(); }
    },
    'reset': function() {
      this.$data.scale = 1.;
      this.$data.offsetX = 0;
      this.$data.offsetY = 0;
      this.$data.raw = false;
      this.$data.preview = false;
      this.update();
    }
  },
  watch: {
    value(new_val) {
      this.$data.scale = new_val;
      this.$data.offsetX = this.$data.view_x * this.$data.scale;
      this.$data.offsetY = this.$data.view_y * this.$data.scale;
      this.update();
    },
    x(new_val) {
      var canvas_w = 1920.;
      this.$data.view_x = (new_val - canvas_w/2. + this.$data.offsetX) / this.$data.scale;
      this.$data.view_scale = this.$data.scale;
    },
    y(new_val) {
      var canvas_h = 1080.;
      this.$data.view_y = (new_val - canvas_h/2. + this.$data.offsetY) / this.$data.scale;
      this.$data.view_scale = this.$data.scale;
    },
    raw(new_val) {
      this.update();
    },
    preview(new_val) {
      this.update();
    },
  },
  mounted: function() {
    this.viewpoint_endpoint = new StarcryAPI(
        'viewpoint',
        StarcryAPI.json_type,
        msg => { },
        buffer => {
          this.$parent.scale = buffer["scale"];
          this.$data.scale = buffer["scale"];
          this.$data.view_x = buffer["offset_x"] / buffer["scale"];
          this.$data.view_y = buffer["offset_y"] / buffer["scale"];
          this.$data.offsetX = buffer["offset_x"];
          this.$data.offsetY = buffer["offset_y"];
          this.$data.raw = buffer["raw"];
          this.$data.preview = buffer["preview"];
        },
        _ => {
          this.viewpoint_endpoint.send(JSON.stringify({
            'operation': 'read',
          }));
        });
  }
}
</script>

<style scoped>
</style>
