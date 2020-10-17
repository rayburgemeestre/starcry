<template>
  <div style="width: 100%">
      <div v-for="dat in test">
          <b>{{ dat.name }}</b> {{ dat.counter }} <br/>
      </div>
  </div>
</template>

<script>
  let ws;
  let retry;
  export default {
    data() {
      return {
        test: []
      }
    },
    methods: {
      connect: function() {
        let protocol = document.location.protocol.replace('http', 'ws');
        if (document.location.href.indexOf('localhost')) {
          ws = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/stats');
        } else {
          ws = new WebSocket(protocol + '//' + document.location.host + '/stats');
        }
        ws.onopen = function () {
          clearTimeout(retry);
        }.bind(this);
        ws.onclose = function () {
          retry = setTimeout(this.connect, 1000);
        }.bind(this);
        ws.onmessage = function (message) {
            message.data.arrayBuffer().then(function(buffer) {
                let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
                console.log(JSON.parse(str));
                this.$data.test = JSON.parse(str);
            }.bind(this));
        }.bind(this);
        ws.onerror = function (error) {
          console.log("ERROR: " + error, error);
        };
      },
    },
    watch: {
    },
    mounted: function() {
      this.connect();
    }
  }
</script>

<style scoped>
</style>
