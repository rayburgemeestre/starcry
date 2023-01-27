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
import { useScriptStore } from 'stores/script';

// see: https://github.com/suren-atoyan/monaco-react/issues/408
// and: https://www.npmjs.com/package/coder-vue3-monaco-editor
import editorWorker from 'monaco-editor/esm/vs/editor/editor.worker?worker'
import jsonWorker from 'monaco-editor/esm/vs/language/json/json.worker?worker'
import cssWorker from 'monaco-editor/esm/vs/language/css/css.worker?worker'
import htmlWorker from 'monaco-editor/esm/vs/language/html/html.worker?worker'
import tsWorker from 'monaco-editor/esm/vs/language/typescript/ts.worker?worker'

window.MonacoEnvironment = {
  getWorker(_, label) {
    if (label === 'json') {
      return new jsonWorker()
    }
    if (label === 'css' || label === 'scss' || label === 'less') {
      return new cssWorker()
    }
    if (label === 'html' || label === 'handlebars' || label === 'razor') {
      return new htmlWorker()
    }
    if (label === 'typescript' || label === 'javascript') {
      return new tsWorker()
    }
    return new editorWorker()
  }
}

let editor: monaco.editor.IStandaloneCodeEditor | null = null;

let script_store = useScriptStore();

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

    editor.onDidChangeModelContent((event: any) => {
      const value = editor?.getValue();
      script_store.set_value(value);
    });
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
