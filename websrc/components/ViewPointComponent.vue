<template>
  <div>
    Scale:
    <b-slider style="width: calc(100%); margin-right: 10px; float: left;" :min="0" :max="100" v-model="scale" ticks></b-slider>
    <br/>
    <label class="checkbox"><input type="checkbox" v-model="auto_render"> <span>auto render</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="raw"> <span>raw</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="preview"> <span>preview</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="labels"> <span>labels</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="save"> <span>save</span></label> <br/>
    <label class="checkbox"><input type="checkbox" v-model="caching"> <span>caching</span></label> <br/>
    <br/>

    <strong>Viewpoint</strong> <br/>
    <span v-for="v, k in viewpoint_settings">
        {{ k }}: {{ v }} <br/>
    </span>
    Scale: {{ scale }} <br/>
    View X: {{ view_x }} <br/>
    View Y: {{ view_y }} <br/>
    View Scale: {{ view_scale }}<br/>
    Offset X: {{ offsetX }} <br/>
    Offset Y: {{ offsetY }} <br/>

    <strong>Video</strong> <br/>
    <span v-for="v, k in video">
        {{ k }}: {{ v }} <br/>
    </span>

    <strong>Preview</strong> <br/>
    <span v-for="v, k in preview_settings">
        {{ k }}: {{ v }} <br/>
    </span>

    <b-button @click="reset">Reset</b-button>

  </div>
</template>

<script>
import StarcryAPI from '../util/StarcryAPI'
var timer = false;
export default {
  props: {
    viewpoint_settings: {
      type: Object,
      required: true
    },
    video: {
      type: Object,
      required: true
    },
    preview_settings: {
      type: Object,
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
      auto_render: false,
      raw: false,
      preview: true,
      labels: false,
      caching: false,
      save: false,
      canvas_w: 0.,
      canvas_h: 0.,
    }
  },
  methods: {
    'select': function() {
      this.$data.offsetX += this.$data.view_x / this.$data.scale;
      this.$data.offsetY += this.$data.view_y / this.$data.scale;
      this.update();
    },
    'update': function() {
      console.log("update");
      if (timer !== false) {
        clearTimeout(timer);
        timer = false;
      }
      timer = setTimeout(this.scheduled_update.bind(this), 50);
      if (!this.$data.labels) this.$parent.reset_labels_canvas();
    },
    'scheduled_update': function() {
      this.viewpoint_endpoint.send(JSON.stringify({
        'operation': 'set',
        'scale': this.$data.scale,
        'offset_x': this.$data.offsetX,
        'offset_y': this.$data.offsetY,
        'raw': this.$data.raw,
        'preview': this.$data.preview,
        'labels': this.$data.labels,
        'caching': this.$data.caching,
        'save': this.$data.save,
        'canvas_w': this.$data.canvas_w,
        'canvas_h': this.$data.canvas_h,
      }));

      // communicate offset_x and offset_y back to parent.
      this.$parent.set_offset(this.$data.offsetX, this.$data.offsetY);

      if (this.$data.scale === this.$data.previous_scale) return;
      // Delay might not be needed, just want to be sure the viewpoint send above is received before the render request.
      if (this.$data.auto_render) { setTimeout(function(){ this.$parent.set_frame(); }.bind(this), 10); }
      this.$data.previous_scale = this.$data.scale;
    },
    'reset': function() {
      this.$data.scale = 1.;
      this.$data.offsetX = 0;
      this.$data.offsetY = 0;
      this.$data.raw = false;
      this.$data.preview = false;
      this.$data.labels = false;
      this.$data.caching = true;
      this.$data.save = false;
      this.update();
    }
  },
  watch: {
    viewpoint_settings: {
        handler: function(new_val) {
          this.$data.scale = new_val.scale;
          this.$data.canvas_w = new_val.canvas_w;
          this.$data.canvas_h = new_val.canvas_h;
          this.$data.view_x = (new_val.view_x - this.$data.canvas_w/2.);
          this.$data.view_y = (new_val.view_y - this.$data.canvas_h/2.);
          this.update();
        },
        deep: true,
    },
    raw(new_val) {
      this.update();
    },
    preview(new_val) {
      this.update();
    },
    save(new_val) {
      this.update();
    },
    labels(new_val) {
      this.update();
    },
    caching(new_val) {
      this.update();
    },
    scale(new_val) {
      this.update();
    }
  },
  mounted: function() {
    this.viewpoint_endpoint = new StarcryAPI(
        'viewpoint',
        StarcryAPI.json_type,
        msg => { },
        buffer => {
          this.$parent.scale = buffer["scale"];
          this.$data.scale = buffer["scale"];
          // this.$data.view_x = buffer["offset_x"] / buffer["scale"];
          // this.$data.view_y = buffer["offset_y"] / buffer["scale"];
          // this.$data.offsetX = buffer["offset_x"];
          // this.$data.offsetY = buffer["offset_y"];
          this.$data.raw = buffer["raw"];
          this.$data.preview = buffer["preview"];
          this.$data.save = buffer["save"];
          this.$data.labels = buffer["labels"];
          this.$data.caching = buffer["caching"];
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
