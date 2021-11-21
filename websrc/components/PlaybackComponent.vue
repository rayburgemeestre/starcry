<template>
  <div style="width: 100%">
    <b-slider style="width: calc(100% - 160px); margin-right: 10px; float: left;" :min="0" v-bind:max="max" v-model="tick" ticks></b-slider>
    <b-button @click="stop">Stop</b-button>
    <b-button @click="play">Play</b-button>
  </div>
</template>

<script>
  export default {
    props: {
      value: {
        type: Number,
        required: true
      },
      max_frames: {
        type: Number,
        required: true
      },
    },
    data() {
      return {
        tick: 0,
        max: 250,
      }
    },
    methods: {
      play : function () {
        this.$parent.play();
      },
      stop : function () {
        this.$parent.stop();
      }
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
    },
    mounted: function() {
      this.$data.tick = this.$props.value;
    }
  }
</script>

<style scoped>
</style>
