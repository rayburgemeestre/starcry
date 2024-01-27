<template>
  <div style="width: 100%">
    <div class="myslider" style="width: 100%; margin-right: 10px; float: left" :min="0" @mouseout="mouse_out">
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
        // trigger render of this frame
        script_store.render_requested_by_user_v2++;
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
      let copy = JSON.parse(JSON.stringify(script_store.frames_per_scene));
      let current_scene = copy.shift();
      let result: any[] = [];
      if (typeof current_scene == 'undefined') {
        return result;
      }
      let color_idx = 0;
      for (let i = 0; i < this.max; i++) {
        if (i >= current_scene) {
          current_scene += copy.shift() as number;
          color_idx++;
          color_idx = color_idx % colors.length;
        }
        if (script_store.job_skipped != -1 && script_store.job_skipped >= i) {
          result.push([i, 'skipped ' + colors[color_idx]]);
        } else if (script_store.job_rendered != -1 && script_store.job_rendered == i) {
          result.push([i, 'rendered']);
        } else if (script_store.job_rendering != -1 && script_store.job_rendering == i) {
          result.push([i, 'rendering']);
        } else if (script_store.frame == i) {
          result.push([i, 'current']);
        } else if (this.tick == i) {
          result.push([i, 'selected']);
        } else {
          result.push([i, colors[color_idx]]);
        }
      }
      return result;
    },
  },
});
</script>

<style scoped>
.myslider {
  width: 100%;
  display: -webkit-flex; /* Safari */
  display: flex; /* Standard syntax */
}
span.info {
  position: absolute;
  top: calc(100% - 40px);
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
.myslider div.skipped {
  mix-blend-mode: color-burn;
}
.myslider div.current {
  background-color: #0000ff;
}
.myslider div.rendering {
  background-color: #ff00ff;
}
.myslider div.rendered {
  background-color: #00ff00;
}
.myslider div.queued {
  background-color: #ffff00;
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
