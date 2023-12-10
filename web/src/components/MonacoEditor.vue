<template>
  <div>
    <div class="container" :id="name"></div>
    <div class="tags">
      <q-checkbox dark v-model="vim_mode" label="vim" color="#990000" />
      <q-checkbox dark v-model="emacs_mode" label="emacs" color="#990000" />
      <br />
      <span class="status" :id="`${name}_status`"></span>
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, watch } from 'vue';
import * as monaco from 'monaco-editor';
import { useScriptStore } from 'stores/script';

// see: https://github.com/suren-atoyan/monaco-react/issues/408
// and: https://www.npmjs.com/package/coder-vue3-monaco-editor
import editorWorker from 'monaco-editor/esm/vs/editor/editor.worker?worker';
import jsonWorker from 'monaco-editor/esm/vs/language/json/json.worker?worker';
import cssWorker from 'monaco-editor/esm/vs/language/css/css.worker?worker';
import htmlWorker from 'monaco-editor/esm/vs/language/html/html.worker?worker';
import tsWorker from 'monaco-editor/esm/vs/language/typescript/ts.worker?worker';
import { initVimMode } from 'monaco-vim/lib';
import { EmacsExtension } from 'monaco-emacs';

window.MonacoEnvironment = {
  getWorker(_, label) {
    if (label === 'json') {
      return new jsonWorker();
    }
    if (label === 'css' || label === 'scss' || label === 'less') {
      return new cssWorker();
    }
    if (label === 'html' || label === 'handlebars' || label === 'razor') {
      return new htmlWorker();
    }
    if (label === 'typescript' || label === 'javascript') {
      return new tsWorker();
    }
    return new editorWorker();
  },
};

let editor: monaco.editor.IStandaloneCodeEditor | null = null;

let script_store = useScriptStore();

export default defineComponent({
  name: 'MonacoEditor',
  props: {
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
  setup() {
    return {
      vim_mode: ref(true),
      emacs_mode: ref(false),
    };
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

    let vimmode = null;
    let name = this.name;
    function toggle_vim_mode(value: boolean) {
      if (value) {
        this.emacs_mode = false;
        vimmode = initVimMode(
          editor,
          document.getElementById(name + '_status')
        );
      } else {
        vimmode?.dispose();
        document.getElementById(name + '_status').innerHTML = '';
      }
    }
    let emacsMode = null;
    function toggle_emacs_mode(value: boolean) {
      let statusNode = document.getElementById(name + '_status');
      if (value) {
        this.vim_mode = false;
        emacsMode = new EmacsExtension(editor);
        emacsMode.onDidMarkChange(function (ev) {
          statusNode.textContent = ev ? 'Mark Set!' : 'Mark Unset';
        });
        emacsMode.onDidChangeKey(function (str) {
          statusNode.textContent = str;
        });
        emacsMode.start();
      } else {
        emacsMode?.dispose();
        document.getElementById(name + '_status').innerHTML = '';
      }
    }
    watch(
      () => this.vim_mode,
      function (val) {
        toggle_vim_mode.bind(this)(val);
      }.bind(this)
    );
    watch(
      () => this.emacs_mode,
      function (val) {
        toggle_emacs_mode.bind(this)(val);
      }.bind(this)
    );
    if (this.vim_mode) toggle_vim_mode.bind(this)(true);
    if (this.emacs_mode) toggle_emacs_mode.bind(this)(true);

    // TODO: this is not really the place to do this, can we add API to a component?
    // Need to figure out the "vue3"/quasar way to do things.
    let script = useScriptStore();
    watch(
      () => script.script,
      function (val) {
        editor?.setValue(val);
      }.bind(this)
    );
  },
});
</script>

<style>
/* this hardcodes styling behavior into this component for our intended purposes */
.container {
  width: 100%;
  height: calc(100vh - (9.25rem) - 5.5em);
}

.q-body--prevent-scroll .container {
  height: calc(100vh - 5.5em);
}
.tags {
  height: 5.5em;
  width: 100%;
}
.tags .status {
  font-family: monospace;
  color: #900;
  padding-left: 5px;
  padding-right: 5px;
}
</style>
