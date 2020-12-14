<template>
  <div>
    <b-table focusable :data="data" :columns="columns" :selected.sync="selected"></b-table>
  </div>
</template>

<script>
  import StarcryAPI from '../util/StarcryAPI'

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
    },
    watch: {
      selected: function (new_value) {
        this.$parent.open(new_value['filename']);
      }
    },
    mounted: function() {
      this.script_endpoint = new StarcryAPI(
              'script',
              msg => {
                this.$data.websock_status = msg;
              },
              buffer => {
                let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
                this.$data.data = JSON.parse(str);
              },
              _ => {
                this.script_endpoint.send("open " + this.$data.filename);
                this.script_endpoint.send("list");
              }
      );
    }
  }
</script>

<style scoped>
</style>
