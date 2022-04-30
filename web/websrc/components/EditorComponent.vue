<template>
  <div>
    <div class="container" :id="name" :style="{'width': `${width}`, 'height': `calc(${height} - ( 8rem) )`}"></div>
    <div class="tags has-addons">
      <span class="tag">
        <label class="checkbox">
          <input type="checkbox" v-model="vim_mode"><span>VIM</span>
        </label>
      </span>
      <span class="tag"><label class="checkbox">
        <input type="checkbox" v-model="emacs_mode"><span>EMACS</span></label>
      </span>
      <span class="tag is-primary" :id="`${name}_status`"></span>
    </div>
  </div>
</template>

<script>
    import { initVimMode } from 'monaco-vim';
    import { EmacsExtension } from 'monaco-emacs';
    import * as monaco from 'monaco-editor';

    export default {
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
                required: true
            },
            value: {
                type: String,
                required: false
            },
            language: {
                type: String,
                required: false
            },
            width: {
                type: String,
                required: true
            },
            height: {
                type: String,
                required: true
            }
        },
        mounted() {
            this.editor = monaco.editor.create(document.getElementById(this.name), {
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
            this.editor.onDidChangeModelContent(event => {
                const value = this.editor.getValue();
                if (this.value !== value) {
                    this.$emit('input', value, event)
                }
            });
        },
        watch: {
            value(new_val) {
                if (this.editor) {
                    if (new_val !== this.editor.getValue()) {
                        this.editor.setValue(new_val);
                        this.$emit('input', new_val);
                    }
                }
            },
            vim_mode(new_val) {
                if (new_val) {
                    this.emacs_mode = false;

                    this.vimMode_1 = initVimMode(this.editor, document.getElementById(this.name + "_status"))
                } else {
                    this.vimMode_1.dispose();
                    document.getElementById(this.name + "_status").innerHTML = '';
                }
            },
            emacs_mode(new_val) {
                var statusNode = document.getElementById(this.name + "_status");
                if (new_val) {
                    this.vim_mode = false;
                    this.emacsMode = new EmacsExtension(this.editor);
                    this.emacsMode.onDidMarkChange(function (ev) {
                          statusNode.textContent = ev ? 'Mark Set!' : 'Mark Unset';
                    });
                    this.emacsMode.onDidChangeKey(function (str) {
                          statusNode.textContent = str;
                    });
                    this.emacsMode.start();
                } else {
                    if (this.emacsMode) this.emacsMode.dispose();
                    document.getElementById(this.name + "_status").innerHTML = '';
                }
            }
        }
    }
</script>

<style scoped>
</style>
