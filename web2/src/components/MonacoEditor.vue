<template>
  <div>
    <div
      class="container"
      :id="name"
      :style="{ width: `${width}`, height: `calc(${height} - ( 8rem) )` }"
    ></div>
    <div class="tags has-addons">
      <!--
      <span class="tag">
        <label class="checkbox">
          <input type="checkbox" v-model="vim_mode" /><span>VIM</span>
        </label>
      </span>
      <span class="tag"
      ><label class="checkbox">
          <input type="checkbox" v-model="emacs_mode" /><span
      >EMACS</span
      ></label
      >
      </span>
      -->
      <span class="tag is-primary" :id="`${name}_status`"></span>
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import * as monaco from 'monaco-editor';

export default defineComponent({
  name: 'MonacoEditor',
  props: {
    vim_mode: {
      type: Boolean,
      default: false,
    },
    emacs_mode: {
      type: Boolean,
      default: false,
    },
    name: {
      type: String,
      required: true,
    },
    value: {
      type: String,
      required: false,
    },
    language: {
      type: String,
      required: false,
    },
    width: {
      type: String,
      required: true,
    },
    height: {
      type: String,
      required: true,
    },
  },
  mounted() {
    let editor = monaco.editor.create(document.getElementById(this.name), {
      value: this.value,
      language: this.language,
      automaticLayout: true,
    });
    /*
            The themes seem not compatible with the vim style cursor.
            TODO: investigate how to fix.
            fetch('/themes/Dawn.json')
                .then(data => data.json())
                .then(data => {
                        monaco.editor.defineTheme('custom', data);
                        monaco.editor.setTheme('custom');
                });
            */
    editor.onDidChangeModelContent((event) => {
      const value = editor.getValue();
      if (this.value !== value) {
        this.$emit('input-change', value, event);
      }
    });
  },
});
</script>
