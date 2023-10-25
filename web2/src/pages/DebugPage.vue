<template>
  <div class="q-pa-md">
    <q-table
      dense
      :rows="rows"
      :columns="columns"
      row-key="name"
      :filter="filter"
      hide-pagination
      :rows-per-page-options="[100]"
    >
      <template v-slot:body="props">
        <q-tr :props="props">
          <q-td v-for="col in props.row" :key="col[0]">
            <span v-if="col !== 'BUSY'">{{ col }}</span>
            <q-spinner-rings v-if="col === 'BUSY'" color="primary" size="2em" />
          </q-td>
        </q-tr>
      </template>
    </q-table>
  </div>
  <div class="q-pa-md flex flex-center">
    <q-knob
      :min="5"
      :max="10"
      :inner-min="6"
      v-model="value1"
      show-value
      size="50px"
      :thickness="0.22"
      color="teal"
      track-color="grey-3"
      class="q-ma-md"
    />

    <q-knob
      :min="55"
      :max="90"
      :inner-min="70"
      :inner-max="85"
      v-model="value2"
      show-value
      size="50px"
      :thickness="0.22"
      color="teal"
      track-color="grey-3"
      class="q-ma-md"
    />

    <q-knob
      :min="40"
      :max="110"
      :inner-min="50"
      :inner-max="100"
      v-model="value3"
      show-value
      size="50px"
      :thickness="0.22"
      color="teal"
      track-color="grey-3"
      class="q-ma-md"
    >
      T1<br />
      {{ value3 }}
    </q-knob>

    <q-knob
      :min="20"
      :max="70"
      :inner-min="30"
      :inner-max="60"
      v-model="value4"
      show-value
      size="50px"
      :thickness="0.22"
      color="teal"
      track-color="grey-3"
      class="q-ma-md"
    />

    <q-knob
      :inner-max="75"
      v-model="value5"
      show-value
      size="50px"
      :thickness="0.22"
      color="teal"
      track-color="grey-3"
      class="q-ma-md"
    />
  </div>

  <div>TODO debug page</div>
  <div>{{ status }}</div>
  <pre v-for="msg in msgs" :key="msg.type">{{ pretty(msg) }}<hr></pre>
</template>

<script lang="ts">
import { defineComponent, ref } from 'vue';
import { StarcryAPI } from 'components/api';
export default defineComponent({
  name: 'DebugPage',
  setup() {
    let status = ref('');
    let msgs = ref({});
    let rows = ref([]);

    const stats_endpoint = new StarcryAPI(
      'stats',
      StarcryAPI.json_type,
      (msg: string) => {
        status.value = msg;
      },
      (buffer: string) => {
        msgs.value[buffer.type] = buffer;
        if (buffer.type === 'metrics') {
          rows.value = [];
          for (let thread of buffer.data.threads) {
            console.log(thread);
            console.log([thread.name, thread.seconds_idle, thread.state]);
            rows.value.push([
              thread.name,
              Math.round(thread.seconds_idle),
              thread.state,
            ]);
          }
        }
      }
    );

    function pretty(value) {
      return JSON.stringify(value, null, 2);
    }

    return {
      status,
      msgs,
      pretty,
      value1: ref(7),
      value2: ref(70),
      value3: ref(80),
      value4: ref(35),
      value5: ref(70),
      rows,
      columns: [
        {
          name: 'thread',
          label: 'thread',
          align: 'left',
          field: (row) => row[0],
          format: (val) => `${val}`,
          sortable: true,
        },
        {
          name: 'seconds_idle',
          label: 'seconds_idle',
          align: 'right',
          field: (row) => row[1],
          sortable: true,
        },
        {
          name: 'status',
          label: 'status',
          align: 'left',
          field: (row) => row[2],
          sortable: true,
        },
      ],
    };
  },
});
</script>
