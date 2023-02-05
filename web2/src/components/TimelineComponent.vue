<template>
  <div style="width: 100%">
    <div
      class="myslider"
      style="width: 100%; margin-right: 10px; float: left"
      :min="1"
      @mouseout="mouse_out"
    >
      <div
        v-bind:class="t"
        v-bind:key="index"
        v-bind:index="index"
        @click="click"
        @mouseover="mouse_over"
        v-for="[index, t] in computed_ticks"
      ></div>
    </div>
    <span class="info"
      ><span>{{ script_store.frame }}</span
      ><span v-if="hover_value !== -1"> &lt;= {{ hover_value }} </span></span
    >
  </div>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { useScriptStore } from 'stores/script';

let script_store = useScriptStore();

let tick = 0;
let max = 250;
let hover_val = -1;

export default defineComponent({
  name: 'TimelineComponent',
  setup() {
    return {
      script_store,

      tick: ref(tick),
      max: ref(max),
      hover_value: ref(hover_val),
      play: function () {
        // this.$parent.play();
      },
      stop: function () {
        // this.$parent.stop();
      },
      click: function (e) {
        let index = e.target.getAttribute('index');
        script_store.frame = index;
        tick = index;
      },
      mouse_over: function (e) {
        hover_val = e.target.getAttribute('index');
      },
      mouse_out: function (e) {
        hover_val = -1;
      },
    };
  },
  computed: {
    computed_ticks() {
      let colors = ['one', 'two'];
      let copy = script_store.frames_per_scene;
      let current_scene = copy.shift();
      let result: any[] = [];
      if (typeof current_scene == 'undefined') {
        return result;
      }
      let color_idx = 0;
      for (let i = 1; i < this.max; i++) {
        if (i >= current_scene) {
          current_scene += copy.shift() as number;
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
