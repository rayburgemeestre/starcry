<template>
  <div>
    <b-table focusable :data="data" :columns="columns" :selected.sync="selected"></b-table>
  </div>
</template>

<script>
  let ws;
  let retry;
  export default {
    data() {
      return {
        data: [
          // { 'filename': 'hello.js', 'filesize': '10kb', 'modified': '2016-10-15 13:43:27'},
        ],
        columns: [
          {
            field: 'filename',
            label: 'File Name',
          },
          {
            field: 'filesize',
            label: 'File Size',
          },
          {
            field: 'modified',
            label: 'Modified',
          },
        ],
        selected: null,
        websock_status: "",
      }
    },
    methods: {
      connect: function() {
        this.$data.websock_status = 'connecting';
        let protocol = document.location.protocol.replace('http', 'ws');
        if (document.location.href.indexOf('localhost')) {
          ws = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/script', ['tag_test']);
        } else {
          ws = new WebSocket(protocol + '//' + document.location.host + '/script', ['tag_test']);
        }
        ws.onopen = function () {
          clearInterval(retry);
          this.$data.websock_status = 'connected';
          ws.send("list");
        }.bind(this);
        ws.onclose = function () {
          this.$data.websock_status = 'disconnected';
          retry = setTimeout(this.connect, 1000);
          console.log(ws);
        }.bind(this);
        ws.onmessage = function (message) {
          message.data.arrayBuffer().then(function(buffer) {
            let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
            console.log(JSON.parse(str));
            this.$data.data = JSON.parse(str);
          }.bind(this));
        }.bind(this);
        ws.onerror = function (error) {
          console.log("ERROR: " + error);
        };
      }
    },
    watch: {
      selected: function (new_value) {
        this.$parent.open(new_value['filename']);
      }
    },
    mounted: function() {
      this.connect();
    }
  }
</script>

<style scoped>
</style>
