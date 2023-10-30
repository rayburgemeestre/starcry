<template>
  <ObjectBrowser />
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import ObjectBrowser from 'components/ObjectBrowser.vue';
export default defineComponent({
  name: 'ObjectsPage',
  components: { ObjectBrowser },
  setup() {
    return {};
  },
});
</script>
