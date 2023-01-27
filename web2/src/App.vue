<template>
  <router-view />
</template>

<script lang="ts">
import { watch, defineComponent } from 'vue';
import { StarcryAPI } from 'components/api';
import { JsonWithObjectsParser } from 'components/json_parser';
import { create_tree, sort_tree } from 'components/filetree';
import { useScriptStore } from 'stores/script';
import { useFilesStore } from 'stores/files';
import { useViewpointStore } from 'stores/viewpoint';

// temp
let timeout_script_updates: NodeJS.Timeout | null = null;

export default defineComponent({
  name: 'App',
  setup() {
    let script_store = useScriptStore();
    let files_store = useFilesStore();
    let viewpoint_store = useViewpointStore();

    // user has made changes to the project javascript through the editor
    watch(
      () => script_store.value_updated_by_user,
      () => {
        if (timeout_script_updates) clearTimeout(timeout_script_updates);
        timeout_script_updates = setTimeout(
          function () {
            script_endpoint.send('set ' + script_store.script);
          }.bind(this),
          1000
        );
      }
    );

    // different script has been selected in the tree
    watch(
      () => files_store.selected,
      () => {
        script_endpoint.send('open ' + files_store.selected);
        script_store.filename = files_store.selected;
      }
    );

    const script_endpoint = new StarcryAPI(
      'script',
      StarcryAPI.text_type,
      (msg: string) => {
        console.log(msg);
      },
      (buffer: string) => {
        if (buffer[0] === '1') {
          script_endpoint.send('open ' + buffer.slice(1));
        } else if (buffer[0] === '2') {
          script_store.script = buffer.slice(1);

          let p = new JsonWithObjectsParser(buffer.substr(buffer.indexOf('{')));
          // this.$data.input_source = buffer;
          script_store.video = p.parsed()['video'] || {};
          script_store.preview = p.parsed()['preview'] || {};
          viewpoint_store.scale = script_store.video['scale'] || 1.;
          let total_duration = 0;
          script_store.frames_per_scene = [];
          for (let scene of p.parsed()['scenes']) {
            if (scene.duration) {
              total_duration += scene.duration;
            }
            script_store.frames_per_scene.push(
              scene.duration * script_store.video['fps']
            );
          }
          if (!total_duration) total_duration = script_store.video['duration'];
          script_store.max_frames = Math.floor(
            total_duration * script_store.video['fps']
          );
        } else if (buffer[0] === '3') {
          script_store.filename = buffer.slice(1);
        } else if (buffer[0] === '4') {
          // traverse tree and sort all children
          let tree = create_tree(buffer.slice(1));
          sort_tree(tree);
          files_store.simple = tree;
        }
      },
      (_) => {
        script_endpoint.send('list');
      }
    );

    return {};
  },
});
</script>
