<template>SCRIPT: {{ script }}</template>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
  name: 'MonacoEditor',
  props: {
    script: {
      type: String,
      required: true,
    },
  },
});
</script>
