export interface ScriptFile {
  filename: string;
  filesize: number;
  modified: string;
}

export interface TreeNode {
  key: string;
  label: string;
  children?: TreeNode[];
}

export function create_tree(files_list_json_string: string): TreeNode[] {
  const tree: TreeNode[] = [];
  const files: ScriptFile[] = JSON.parse(files_list_json_string);
  for (const file of files) {
    const parts = file.filename.split('/');
    const parents = [];
    for (let i = 0; i < parts.length; i++) {
      const part = parts[i];
      const is_file = i == parts.length - 1;
      parents.push(part);
      // ensure we have this directory in the tree
      let current: TreeNode[] = tree;
      for (const parent of parents) {
        // see if current is in current
        let item: TreeNode | undefined = current.find((node) => node.label == parent);
        let found: boolean = item != undefined;
        if (!found) {
          const new_children: TreeNode[] = [];
          if (!is_file) {
            current.push({
              key: parent,
              label: parent,
              children: new_children,
            });
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
        current.push({ key: file.filename, label: part });
      }
    }
  }
  console.log(tree);
  return tree;
}

export function sort_tree(tree: TreeNode[]) {
  for (const node of tree) {
    if (node.children) {
      node.children.sort((a, b) => a.label.localeCompare(b.label));
      sort_tree(node.children);
    }
  }
}
