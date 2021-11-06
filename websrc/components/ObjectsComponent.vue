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
            field: 'unique_id',
            label: 'Unique ID',
          },
          {
            field: 'id',
            label: 'ID',
          },
          {
            field: 'label',
            label: 'Label',
          },
          {
            field: 'level',
            label: 'Level',
          },
          {
            field: 'index',
            label: 'Index',
          },
          {
            field: 'type',
            label: 'Type',
          },
          {
            field: 'gradient',
            label: 'Gradient',
          },
          {
            field: 'x',
            label: 'X',
          },
          {
            field: 'y',
            label: 'Y',
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
