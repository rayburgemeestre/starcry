<template>
  <div>
    <b-table focusable :data="data" :columns="columns" :selected.sync="selected"></b-table>
  </div>
</template>

<script>
  export default {
    props: {
      value: {
        type: String,
        required: false
      },
    },
    data() {
      return {
        data: [
        ],
        columns: [
          {
            field: 'timestamp',
            label: 'Timestamp',
          },
          {
            field: 'level',
            label: 'Level',
          },
          {
            field: 'api',
            label: 'API',
          },
          {
            field: 'message',
            label: 'message',
          },
          {
            field: 'data',
            label: 'Data',
          },
        ],
        selected: null,
        websock_status: "",
      }
    },
    methods: {
    },
    watch: {
      value: function (new_value) {
        this.$data.data = new_value;
      },
      selected: function (new_value) {
      }
    },
    mounted: function() {
      this.$data.data = this.$props.value;
    }
  }
</script>

<style scoped>
</style>
