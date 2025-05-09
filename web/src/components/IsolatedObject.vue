<template>
  <div class="isolated-object-container">
    <q-card flat bordered class="isolated-object-card">
      <q-card-section class="row items-center">
        <div class="text-h6">Isolated Object Details</div>
        <q-space />
        <q-btn color="negative" dense size="sm" label="Close" @click="onClose" />
      </q-card-section>
      <q-separator />
      <q-card-section style="max-height: 300px" class="scroll">
        <q-list dense>
          <q-item v-for="(value, key) in objectProperties" :key="key">
            <q-item-section style="max-width: 200px">
              <q-item-label class="text-weight-bold">{{ key }}</q-item-label>
            </q-item-section>
            <q-item-section>
              <q-input v-model="editedObject[key]" dense outlined @change="updateObjectProperty(key)" />
            </q-item-section>
          </q-item>
        </q-list>
      </q-card-section>
    </q-card>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, computed, PropType } from 'vue';
import { useScriptStore } from 'stores/script';

interface ObjectData {
  unique_id: number;
  id: string;
  type: string;
  level: number;
  [key: string]: unknown;
}

export default defineComponent({
  name: 'IsolatedObject',
  props: {
    selectedObject: {
      type: Object as PropType<ObjectData>,
      required: true,
    },
  },
  emits: ['close', 'update:selectedObject'],
  setup(props, { emit }) {
    const script_store = useScriptStore();
    const editedObject = ref<Record<string, unknown>>({});

    // Initialize editedObject whenever selectedObject changes
    const initializeEditedObject = () => {
      editedObject.value = { ...props.selectedObject };
    };

    // Call initialization immediately
    initializeEditedObject();

    const objectProperties = computed(() => {
      if (!props.selectedObject) return {};

      // Filter out utility properties we don't want to show
      const excludeProperties = ['tree', 'actions', 'stack'];
      const result: Record<string, unknown> = {};

      // Order important properties first
      const orderingDetail = ['unique_id', 'id', 'type', 'level', 'x', 'y', 'z', 'r', 'g', 'b', 'a'];

      // First add ordered properties
      for (const prop of orderingDetail) {
        if (prop in props.selectedObject && !excludeProperties.includes(prop)) {
          result[prop] = props.selectedObject[prop];
        }
      }

      // Then add remaining properties
      for (const [key, value] of Object.entries(props.selectedObject)) {
        if (!excludeProperties.includes(key) && !orderingDetail.includes(key)) {
          result[key] = value;
        }
      }

      return result;
    });

    const updateObjectProperty = (key: string) => {
      if (props.selectedObject && key in props.selectedObject) {
        // Emit an event to update the parent component's selectedObject
        const updatedObject: ObjectData = {
          ...props.selectedObject,
          [key]: editedObject.value[key],
        };
        emit('update:selectedObject', updatedObject);
        console.log(`Updated property ${key}:`, editedObject.value[key]);
      }
    };

    const onClose = () => {
      emit('close');
      script_store.clearSelectedObjects();
      if (script_store.auto_render) {
        script_store.render_requested_by_user_v2++;
      }
    };

    return {
      editedObject,
      objectProperties,
      updateObjectProperty,
      onClose,
    };
  },
});
</script>

<style scoped>
.isolated-object-container {
  margin-top: 20px;
}
.isolated-object-card {
  width: 100%;
}
.scroll {
  overflow-y: auto;
}
</style>
