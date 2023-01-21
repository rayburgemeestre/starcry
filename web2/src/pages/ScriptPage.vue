<template>
  <monaco-editor name="container" :value="script" language="javascript" />
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import MonacoEditor from 'components/MonacoEditor.vue';
import { StarcryAPI } from 'components/api';

let script = ref('');
const script_endpoint = new StarcryAPI(
  'script',
  StarcryAPI.text_type,
  (msg: string) => {
    console.log(msg);
  },
  (buffer: string) => {
    if (buffer[0] == '1') {
      script_endpoint.send('open ' + buffer.slice(1));
    } else if (buffer[0] == '2') {
      script.value = buffer.slice(1);
    } else if (buffer[0] == '3') {
    } else if (buffer[0] == '4') {
      // script listing
    }
  }
);

export default defineComponent({
  name: 'ScriptPage',
  components: {
    MonacoEditor,
  },
  setup() {
    return { script };
  },
  mounted() {
    script_endpoint.connect();
  },
});
</script>
