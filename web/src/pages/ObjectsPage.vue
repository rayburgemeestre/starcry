<template>
  WIP:
  <ObjectBrowser2 />

  deprecated: <br />
  <ObjectBrowser />
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import ObjectBrowser from 'components/ObjectBrowser.vue';
import ObjectBrowser2 from 'components/ObjectBrowser2.vue';

export default defineComponent({
  name: 'ObjectsPage',
  components: { ObjectBrowser, ObjectBrowser2 },
  setup() {
    return {};
  },
});
</script>
