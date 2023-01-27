<template>
  <monaco-editor
    name="container"
    :value="script.script"
    language="javascript"
  />
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import MonacoEditor from 'components/MonacoEditor.vue';
import { useScriptStore } from 'stores/script';

export default defineComponent({
  name: 'ScriptPage',
  components: {
    MonacoEditor,
  },
  setup() {
    let script = useScriptStore();
    return { script };
  },
});
</script>
