<template>
  <q-table
    dense
    :rows="rows(objects_store.objects)"
    :columns="columns(objects_store.objects)"
    row-key="#"
    hide-header
    hide-pagination
    :rows-per-page-options="[10000]"
  >
  </q-table>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { StarcryAPI } from 'components/api';
import { useObjectsStore } from 'stores/objects';
export default defineComponent({
  name: 'ObjectsPage',
  setup() {
    function columns(objects) {
      let columns = [];
      let index = 0;
      if (objects.length === 0) {
        return columns;
      }
      for (let key in objects[0]) {
        columns.push({
          name: key,
          field: index,
          align: 'left',
          sortable: true,
        });
        index++;
      }
      console.log(columns);
      return columns;
    }
    function rows(objects) {
      let rows = [];
      for (let object of objects) {
        let row = [];
        let found_level = 0;
        for (let field of object) {
          let property = field[0];
          let value = field[1];
          if (property === 'level') found_level = value;
          row.push(value);
        }
        if (found_level === 0) {
          rows.push(row);
        }
      }
      // console.log(rows);
      return rows;
    }
    let objects_store = useObjectsStore();
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    const objects_endpoint = new StarcryAPI(
      'objects',
      StarcryAPI.json_type,
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (msg) => {},
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      (buffer) => {
        objects_store.objects = buffer;
        if (buffer.length) {
          // insert headers as first row in objects_store.objects
          let headers = [];
          for (let field of buffer[0]) {
            headers.push([field[0], field[0]]);
          }
          console.log(headers);
          console.log('OK');
          objects_store.objects.unshift(headers);
        }
        // this.render_objects();
      },
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (_) => {},
      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      // eslint-disable-next-line @typescript-eslint/no-empty-function
      (_) => {}
    );

    return {
      objects_store,
      rows,
      columns,
    };
  },
});
</script>
