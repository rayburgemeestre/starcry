<template>
  <router-view />
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { useCounterStore } from 'stores/example-store';

export default defineComponent({
  name: 'App',
  setup() {
    let counter = useCounterStore();
    counter.$patch({ counter: 1001 });
    setTimeout(() => {
      counter.$patch({ counter: 1001 * 2 });
    }, 1000);
    return {};
  },
});
</script>
