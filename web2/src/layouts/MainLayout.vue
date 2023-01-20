<template>
  <q-layout view="hHh lpR fFf">

    <q-header elevated class="bg-primary text-black" height-hint="98">
      <q-toolbar>
        <q-btn dense flat round icon="menu" @click="toggleLeftDrawer" />

        <q-toolbar-title>
          <img src="sc.png" style="position: relative; top: 10px; left: 10px; height: 40px;">
        </q-toolbar-title>

        <q-btn dense flat round icon="menu" @click="toggleRightDrawer" />
      </q-toolbar>

      <q-tabs align="left">
        <q-route-tab to="/files" label="Files" />
        <q-route-tab to="/script" label="Script" />
        <q-route-tab to="/objects" label="Objects" />
        <q-route-tab to="/help" label="Help" />
        <q-route-tab to="/debug" label="Debug" />
      </q-tabs>
    </q-header>

    <q-drawer show-if-above v-model="leftDrawerOpen" side="left" bordered>
      <router-view />
    </q-drawer>

    <q-drawer show-if-above v-model="rightDrawerOpen" side="right" bordered>
      <!-- drawer content -->
      ATTRIBUTES HERE
    </q-drawer>

    <q-page-container>
      THE CANVAS HERE
    </q-page-container>

    <q-footer elevated class="bg-grey-8 text-white">
      <q-toolbar>
        <q-toolbar-title>
          <div>TIME NAVIGATION HERE</div>
        </q-toolbar-title>
      </q-toolbar>
    </q-footer>

  </q-layout>
</template>

<script>
import { defineComponent, ref } from 'vue';

export default defineComponent({
  name: 'MainLayout',

  components: {},

  setup () {
    const leftDrawerOpen = ref(false)
    const rightDrawerOpen = ref(false)
    const script = ref('Hello world')

    return {
      script,
      leftDrawerOpen,
      toggleLeftDrawer () {
        leftDrawerOpen.value = !leftDrawerOpen.value
      },

      rightDrawerOpen,
      toggleRightDrawer () {
        rightDrawerOpen.value = !rightDrawerOpen.value
      }
    }
  }
});
</script>
