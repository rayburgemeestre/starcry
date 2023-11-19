<template>
  <router-view />
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { create_script_endpoint } from 'components/endpoints/script';
import { create_object_endpoint } from 'components/endpoints/object';
import { create_stats_endpoint } from 'components/endpoints/stats';
import { create_bitmap_endpoint } from 'components/endpoints/bitmap';
import { create_viewpoint_endpoint } from 'components/endpoints/viewpoint';

export default defineComponent({
  name: 'App',
  setup() {
    create_script_endpoint();
    create_object_endpoint();
    create_stats_endpoint();
    create_bitmap_endpoint();
    create_viewpoint_endpoint();
    return {};
  },
});
</script>
