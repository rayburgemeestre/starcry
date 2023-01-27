<template>
  <div style="width: 100%">
    <div
      class="myslider"
      style="width: 100%; margin-right: 10px; float: left"
      :min="0"
      @mouseout="mouse_out"
    >
      <div
        v-bind:class="t"
        v-bind:key="index"
        @click="click"
        @mouseover="mouse_over"
        v-for="[index, t] in computed_ticks"
      ></div>
    </div>
    <span class="info"
      ><span>{{ value }}</span
      ><span v-if="hover_value != -1"> &lt;= {{ hover_value }} </span></span
    >
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';

export default defineComponent({
  name: 'TimelineComponent',
  props: {
    value: {
      type: Number,
      required: true,
    },
    max_frames: {
      type: Number,
      required: true,
    },
    frames_per_scene: {
      type: Array,
      required: true,
    },
  },
  setup() {
    return {
      tick: ref(0),
      max: ref(250),
      hover_value: ref(-1),
    };
  },
  computed: {
    computed_ticks() {
      let colors = ['one', 'two'];
      let copy = this.frames_per_scene;
      let current_scene = copy.shift();
      let color_idx = 0;
      let result = [];
      for (let i = 0; i < this.max; i++) {
        if (i >= current_scene) {
          current_scene += copy.shift();
          color_idx++;
          color_idx = color_idx % colors.length;
        }
        if (this.tick == i) {
          result.push([i, 'selected']);
          continue;
        }
        result.push([i, colors[color_idx]]);
      }
      return result;
    },
  },
});
/*
export default {
  props: {
  },
  data() {
    return {
    };
  },
  computed: {
  },
  methods: {
    play: function () {
      this.$parent.play();
    },
    stop: function () {
      this.$parent.stop();
    },
    click: function (e) {
      let index = e.target.getAttribute('index');
      this.$data.value = index;
      this.$data.tick = index;
    },
    mouse_over: function (e) {
      let index = e.target.getAttribute('index');
      this.$data.hover_value = index;
    },
    mouse_out: function (e) {
      this.$data.hover_value = -1;
    },
  },
  watch: {
    value(new_val) {
      this.$data.tick = this.$props.value;
    },
    max_frames(new_val) {
      this.$data.max = this.$props.max_frames;
    },
    tick: function (frame) {
      this.$parent.set_frame(frame);
    },
    frames_per_scene: function (frames_per_scene) {
      this.$data.frames_per_scene = frames_per_scene;
    },
  },
  mounted: function () {
    this.$data.tick = this.$props.value;
    this.$data.frames_per_scene = this.$props.frames_per_scene;
  },
};
*/
</script>

<style>
.myslider {
  width: 100%;
  display: -webkit-flex; /* Safari */
  display: flex; /* Standard syntax */
}
span.info {
  position: absolute;
  top: calc(100vh - 40px);
  left: 50px;
  color: black;
}
.myslider div {
  -webkit-flex: 1; /* Safari */
  -ms-flex: 1; /* IE 10 */
  flex: 1; /* Standard syntax */
  height: 40px;
  background-color: #c0c0c0;
  cursor: pointer;
}
.myslider div.one {
  background-color: #990000;
}
.myslider div.two {
  background-color: #b9f0de;
}
.myslider div.selected {
  background-color: #ffffff;
}
.myslider div:nth-child(even) {
  opacity: 1;
  filter: brightness(1.2);
}
.myslider div:nth-child(odd) {
  opacity: 1;
}
.myslider div:hover {
  background-color: yellow;
  opacity: 1;
}
</style>
