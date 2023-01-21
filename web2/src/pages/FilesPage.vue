<template>
  <div class="q-pa-md q-gutter-sm">
    <q-input ref="filterRef" filled v-model="filter" label="Filter">
      <template v-slot:append>
        <q-icon
          v-if="filter !== ''"
          name="clear"
          class="cursor-pointer"
          @click="resetFilter"
        />
      </template>
    </q-input>

    <q-tree
      :nodes="simple"
      node-key="label"
      :filter="filter"
      default-expand-all
      dense
      selected-color="primary"
      v-model:selected="selected"
    />
  </div>
</template>

<script lang="ts">
import { defineComponent, Ref, ref } from 'vue';
import { StarcryAPI } from 'components/api';

interface ScriptFile {
  filename: string;
  filesize: number;
  modified: string;
}

interface TreeNode {
  label: string;
  children?: TreeNode[];
}

let simple_tree: Ref<TreeNode[]> = ref([]);

const script_endpoint = new StarcryAPI(
  'script',
  StarcryAPI.text_type,
  (msg: string) => {
    console.log(msg);
  },
  (buffer: string) => {
    if (buffer[0] == '1') {
    } else if (buffer[0] == '2' || buffer[0] == '3') {
    } else if (buffer[0] == '4') {
      // script listing
      function create_tree(files_list_json_string: string): TreeNode[] {
        let tree: TreeNode[] = [];
        let files: ScriptFile[] = JSON.parse(files_list_json_string);
        for (let file of files) {
          console.log(file);
          let parts = file.filename.split('/');
          let parents = [];
          for (let i = 0; i < parts.length; i++) {
            let part = parts[i];
            let is_file = i == parts.length - 1;
            parents.push(part);
            // ensure we have this directory in the tree
            let current: TreeNode[] = tree;
            for (let parent of parents) {
              // see if current is in current
              let item: TreeNode | undefined = current.find(
                (node) => node.label == parent
              );
              let found: boolean = item != undefined;
              if (!found) {
                let new_children: TreeNode[] = [];
                if (!is_file) {
                  current.push({ label: parent, children: new_children });
                }
                // now we should be able to find it
                item = current.find((node) => node.label == parent);
                found = item != undefined;
              }
              if (!item || !item.children) {
                continue;
              }
              current = item.children;
            }
            if (is_file) {
              current.push({ label: part });
            }
          }
        }
        return tree;
      }

      // traverse tree and sort all children
      function sort_tree(tree: TreeNode[]) {
        for (let node of tree) {
          if (node.children) {
            node.children.sort((a, b) => a.label.localeCompare(b.label));
            sort_tree(node.children);
          }
        }
      }
      let tree = create_tree(buffer.slice(1));
      sort_tree(tree);
      simple_tree.value = tree;
    }
  },
  () => {
    script_endpoint.send('list');
  }
);

export default defineComponent({
  name: 'FilesPage',
  setup() {
    const filter = ref('');
    const filterRef = ref(null);

    return {
      filter,
      filterRef,
      selected: ref(''),

      simple: simple_tree,
      resetFilter() {
        filter.value = '';
        filterRef.value.focus();
      },
      mounted() {
        script_endpoint.connect();
      },
    };
  },
});
</script>
