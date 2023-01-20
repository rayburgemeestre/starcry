<template>
  <monaco-editor
    name="container"
    v-model="script"
    height="100vh"
    width="100vh"
  />
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import MonacoEditor from 'components/MonacoEditor.vue';
export default defineComponent({
  name: 'ScriptPage',
  components: {
    MonacoEditor,
  },
  setup() {
    const script = ref('Hello world!');
    return { script };
  },
});
</script>
