<template>
  <div id="help">
    <strong>Keyboard shortcuts</strong>
    <table>
      <tr v-for="h in help" :key="h.input">
        <td v-html="h.input"></td>
        <td>{{ h.desc }}</td>
      </tr>
    </table>
  </div>
</template>

<style>
#help {
  margin: 10px;
}
#help code {
  background-color: #666666;
}
</style>

<script lang="ts">
import { defineComponent } from 'vue';
export default defineComponent({
  name: 'HelpPage',
  setup() {
    const help = [
      {
        input: '<code>ctrl</code> + <code>/</code>',
        desc: 'Current frame render.',
      },
      {
        input: '<code>ctrl</code> + <code>,</code>',
        desc: 'Previous frame and render.',
      },
      {
        input: '<code>ctrl</code> + <code>.</code>',
        desc: 'Next frame and render.',
      },
      {
        input: '<code>ctrl</code> + <code>s</code>',
        desc: 'Open script menu.',
      },
      { input: '<code>ctrl</code> + <code>f</code>', desc: 'Open file menu..' },
      {
        input: '<code>ctrl</code> + <code>o</code>',
        desc: 'Open objects menu.',
      },
      { input: '<code>ctrl</code> + <code>u</code>', desc: 'Open debug menu.' },
      {
        input: '<code>ctrl</code> + <code>h</code>',
        desc: "Open help menu (you're looking at it).",
      },
      {
        input: '<nobr><code>shift</code> + <code>o</code></nobr>&nbsp;',
        desc: 'Get objects information for current frame.',
      },
      {
        input: '<code>ctrl</code> + <code>r</code>',
        desc: 'Reset / Re-open file.',
      },
      {
        input: '<code>ctrl</code> + <code>m</code>',
        desc: 'Toggle mouse pointer on canvas.',
      },
      { input: '<code>ctrl</code> + <code>t</code>', desc: 'Toggle menus.' },
    ];
    return { help };
  },
});
</script>
