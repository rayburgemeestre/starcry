<template>
  <div>
    <b-table focusable :data="data" :columns="columns" :selected.sync="selected"></b-table>
  </div>
</template>

<script>
  export default {
    props: {
      value: {
        type: String,
        required: false
      },
    },
    data() {
      return {
        data: [
          { 'input': '<code>ctrl</code> + <code>/</code>', 'desc':  'Current frame render.' },
          { 'input': '<code>ctrl</code> + <code>,</code>', 'desc':  'Previous frame and render.' },
          { 'input': '<code>ctrl</code> + <code>.</code>', 'desc':  'Next frame and render.' },
          { 'input': '<code>ctrl</code> + <code>s</code>', 'desc':  'Open script menu.' },
          { 'input': '<code>ctrl</code> + <code>f</code>', 'desc':  'Open file menu..' },
          { 'input': '<code>ctrl</code> + <code>o</code>', 'desc':  'Open objects menu.' },
          { 'input': '<code>ctrl</code> + <code>u</code>', 'desc':  'Open debug menu.' },
          { 'input': '<code>ctrl</code> + <code>h</code>', 'desc':  'Open help menu (you\'re looking at it).' },
          { 'input': '<code>shift</code> + <code>o</code>', 'desc':  'Get objects information for current frame.' },
          { 'input': '<code>ctrl</code> + <code>r</code>', 'desc':  'Reset / Re-open file.' },
          { 'input': '<code>ctrl</code> + <code>m</code>', 'desc':  'Toggle mouse pointer on canvas.' },
          { 'input': '<code>ctrl</code> + <code>t</code>', 'desc':  'Toggle menus.' },
        ],
        columns: [
          {
            field: 'input',
            label: 'Keyboard input',
          },
          {
            field: 'desc',
            label: 'Action',
          },
        ],
        selected: null,
      }
    },
  }
</script>

<style scoped>
   * { font-size: 120%; }
</style>
