<template>
  <router-view />
</template>

<script lang="ts">
import { watch, defineComponent } from 'vue';
import { StarcryAPI } from 'components/api';
import { create_tree, sort_tree } from 'components/filetree';
import { useScriptStore } from 'stores/script';
import { useFilesStore } from 'stores/files';

// temp
let timeout_script_updates: NodeJS.Timeout | null = null;

export default defineComponent({
  name: 'App',
  setup() {
    let script_store = useScriptStore();
    let files_store = useFilesStore();

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
