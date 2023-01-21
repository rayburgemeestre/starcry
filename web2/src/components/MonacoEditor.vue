<template>
  <div>
    <div class="container" :id="name"></div>
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

let editor: monaco.editor.IStandaloneCodeEditor | null = null;

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
  },
  mounted() {
    let container = document.getElementById(this.name);
    if (!container) {
      return;
    }
    editor = monaco.editor.create(container, {
      value: this.value,
      language: this.language,
      automaticLayout: true,
      overviewRulerLanes: 0,
      hideCursorInOverviewRuler: true,
      scrollbar: {
        vertical: 'hidden',
      },
      overviewRulerBorder: false,
      theme: 'vs-dark',
    });

    // editor.onDidChangeModelContent((event: any) => {
    //   const value = editor?.getValue();
    //   if (this.value !== value) {
    //     this.$emit('input-change', value, event);
    //   }
    // });
  },
  watch: {
    value(value) {
      if (editor) {
        editor.setValue(value);
      }
    },
  },
});
</script>

<style>
/* this hardcodes styling behavior into this component for our intended purposes */
.container {
  width: 100%;
  height: calc(100vh - (9.25rem));
}

.q-body--prevent-scroll .container {
  height: 100vh;
}
</style>
