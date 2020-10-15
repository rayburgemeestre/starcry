<template>
  <div style="width: 100%">
    <b-slider style="width: calc(100% - 160px); margin-right: 10px; float: left;" :min="0" :max="250" v-model="ticks" ticks></b-slider>
    <b-button @click="stop">Stop</b-button>
    <b-button @click="play">Play</b-button>
  </div>
</template>

<script>
  let ws;
  let retry;
  export default {
    data() {
      return {
        ticks: 0,
        websock_status: "",
      }
    },
    methods: {
      connect: function() {
        this.$data.websock_status = 'connecting';
        let protocol = document.location.protocol.replace('http', 'ws');
        if (document.location.href.indexOf('localhost')) {
          ws = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/bitmap', ['tag_test']);
        } else {
          ws = new WebSocket(protocol + '//' + document.location.host + '/bitmap', ['tag_test']);
        }
        ws.onopen = function () {
          clearTimeout(retry);
          this.$data.websock_status = 'connected';
        }.bind(this);
        ws.onclose = function () {
          this.$data.websock_status = 'disconnected';
          retry = setTimeout(this.connect, 1000);
        }.bind(this);
        ws.onmessage = function (message) {
          const reader = new FileReader();
          reader.addEventListener('loadend', (e) => {
            const text = e.srcElement.result;
            console.log(text.length);
            Module.set_texture(text);
          });
          reader.addEventListener('error', (e) => {
            console.log(e);
          });
          //reader.readAsText(message.data);
          // Module.set_texture(message.data);
          message.data.arrayBuffer().then(buffer => {
            Module.set_texture(buffer);
          });

        };
        ws.onerror = function (error) {
          console.log("ERROR: " + error);
        };
      },
      play : function () {
        for (let i=0; i<250; i++) {
          this.$parent.queue_frame(i);
        }
        this.$data.ticks = 250;
      },
      stop : function () {
        this.$parent.stop();
      }
    },
    watch: {
      ticks: function (frame) {
        this.$parent.queue_frame(frame);
      },
    },
    mounted: function() {
      this.connect();
    }
  }
</script>

<style scoped>
</style>
