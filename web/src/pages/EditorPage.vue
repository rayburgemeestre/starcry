<template>
  <div class="q-pa-md">
    <q-list bordered padding>
      <gradients-component @save-changes="save_changes" />

      <q-separator spaced />
      <div class="row items-center">
        <q-item-label header>Objects</q-item-label>
        <q-btn flat dense icon="add" @click="addObject(null)" style="margin-top: -10px" />
      </div>

      <object-component
        :objects="objects"
        :initialSelected="selected"
        @update:objects="objects = $event"
        @object-selected="update_selected($event)"
        @save-changes="save_changes"
      />

      <q-separator spaced />

      <div class="row items-center">
        <q-item-label header>Video</q-item-label>
        <q-btn flat dense icon="add" @click="addVideoProperty" style="margin-top: -10px" />
      </div>

      <q-item v-if="video_adding_property">
        <q-select v-model="video_property" :options="video_properties" label="Video Property" class="full-width" />
      </q-item>
      <q-item v-if="video_adding_property">
        <q-btn flat dense @click="addVideoProperty2" style="margin-top: -10px"> Add Video Property </q-btn>
      </q-item>

      <q-item clickable v-ripple v-for="name in Object.keys(video)" :key="name" dense>
        <template v-if="name === 'bg_color'">
          <q-input
            v-for="colorKey in ['r', 'g', 'b', 'a']"
            :key="colorKey"
            v-model.number="video[name][colorKey]"
            :label="colorKey"
            type="number"
            min="0"
            max="1"
            step="0.1"
          />
        </template>
        <template v-else-if="typeof video[name] === 'boolean'">
          <q-checkbox v-model="video[name]" :label="name" />
        </template>
        <q-input
          v-else
          v-model="video[name]"
          :label="name"
          :type="typeof video[name] === 'number' ? 'number' : 'text'"
        />
      </q-item>

      <q-separator spaced />
      <div class="row items-center">
        <q-item-label header>Scenes</q-item-label>
        <q-btn flat dense icon="add" @click="addScene" style="margin-top: -10px" />
      </div>

      <q-expansion-item
        v-for="(scene, index) in scenes"
        :key="index"
        :label="scene.name"
        header-class="bg-grey-9"
        expand-separator
      >
        <q-card>
          <q-card-section>
            <div class="row q-col-gutter-md">
              <div class="col-6">
                <q-input v-model="scene.name" label="Scene name" dense />
              </div>
              <div class="col-6">
                <q-input
                  v-model.number="scene.duration"
                  label="Duration (seconds)"
                  type="number"
                  min="0"
                  step="0.1"
                  dense
                />
              </div>
            </div>

            <q-separator spaced />

            <div class="row items-center">
              <q-item-label header>Objects</q-item-label>
              <q-btn flat dense icon="add" @click="addObject(scene)" style="margin-top: -10px" />
            </div>

            <object-component
              :objects="scene.objects"
              :initialSelected="selected"
              @update:objects="objects = $event"
              @object-selected="selected = $event"
              @save-changes="save_changes"
            />
          </q-card-section>

          <q-card-actions align="right">
            <q-btn flat color="negative" label="Delete Scene" @click="removeScene(index)" />
          </q-card-actions>
        </q-card>
      </q-expansion-item>

      <q-separator spaced />
      <!-- TODO: Here should be a component for scenes editing -->
      <!-- This is what the scenes array looks like: 
      [
        {
          'name': 'scene1',
          'duration': 3.0,
          'objects':
              [{'id': 'obj0', 'x': 5, 'y': 0, 'z': 0, 'props': {}}, {'id': 'obj0', 'x': -5, 'y': 0, 'z': 0, 'props': {}}]
        },
        {'name': 'scene2', 'duration': 1.0, 'objects': []},
        {'name': 'scene3', 'duration': 1.0, 'objects': []},
      ]
      -->

      <q-item>
        <q-btn color="primary" label="Update" @click="save_changes(null)"></q-btn>
      </q-item>
    </q-list>
  </div>
</template>

<script lang="ts">
import { defineComponent, ref, watch, computed } from 'vue';
import GradientsComponent from 'components/GradientsComponent.vue';
import ObjectComponent from 'components/ObjectComponent.vue';
import { useScriptStore } from 'stores/script';
import { useProjectStore } from 'stores/project';
import { useObjectsStore } from 'stores/objects';
import { ObjectDef, Scene } from 'src/types/types';

const format_cell_value = (value: any): string => {
  return typeof value === 'object' ? JSON.stringify(value) : value;
};

const update_cell_value = (newValue: string, row: [string, any]): void => {
  try {
    if (typeof row[1] === 'object' || newValue.trim().startsWith('{') || newValue.trim().startsWith('[')) {
      row[1] = JSON.parse(newValue);
    } else {
      row[1] = newValue;
    }
  } catch (e) {
    row[1] = newValue;
  }
};

export default defineComponent({
  name: 'EditorPage',
  components: {
    GradientsComponent,
    ObjectComponent,
  },
  setup() {
    let script_store = useScriptStore();
    let project_store = useProjectStore();
    let object_store = useObjectsStore();

    let parsed = project_store.parser?.parsed();
    let gradients = ref<Record<string, any>>(parsed && 'gradients' in parsed ? parsed['gradients'] : {});
    let objects = ref<Record<string, ObjectDef[]>>(parsed && 'objects' in parsed ? parsed['objects'] : {});
    let video = ref<Record<string, any>>(parsed && 'video' in parsed ? parsed['video'] : {});
    let scenes = ref<Array<Scene>>(parsed && 'scenes' in parsed ? parsed['scenes'] : []);

    // Functions for scene management
    function addScene() {
      scenes.value.push({
        name: `Scene ${scenes.value.length + 1}`,
        duration: 2.0,
        objects: [],
      });
    }

    function removeScene(index: number) {
      if (confirm('Are you sure you want to delete this scene?')) {
        scenes.value.splice(index, 1);
      }
    }

    let selected = ref('');

    const columns = [
      {
        name: 'property',
        align: 'left',
        field: (row) => row[0],
        sortable: true,
      },
      {
        name: 'value',
        align: 'left',
        field: (row) => row[1],
        sortable: true,
      },
    ];

    const update_selected = (index: number) => {
      // selected = $event;
      console.log('RBU: update_selected');
      selected.value = index.toString();
      console.log('selected.value: ', selected.value);
      // updating selected object
      object_store.set_selected(selected.value, objects.value[selected.value]);
      console.log('selected.value:', selected.value);
      console.log('object_store.selected: ', objects.value[selected.value]);
      console.log('objects:', objects.value);
    };

    const save_changes = (_gradients?: any) => {
      if (_gradients) {
        gradients.value = _gradients.value;
      }
      let script = 'ERROR: parser is null';
      if (project_store.parser) {
        const parsed = project_store.parser.parsed();
        if (parsed) {
          parsed.gradients = gradients.value;
          parsed.video = video.value;
          parsed.scenes = scenes.value;
        }
        script = project_store.parser.to_string();
      }
      // for internal
      script_store.set_value(script, true);
      // for debugger
      script_store.set_result(script, true);
      // updating selected object
      object_store.set_selected(objects[selected.value]);
    };

    function addObject(scene: Scene | null) {
      const new_obj: ObjectDef = {
        id: 'object',
        x: 0,
        y: 0,
        z: 0,
        props: {},
      };
      if (scene) {
        console.log(scene);
        scene.objects.push(new_obj);
      } else {
        console.log('no context');
        objects.value[new_obj['id']] = new_obj;
      }
    }

    function addVideoProperty() {
      console.log('Add video property clicked');
      script_store.request_video_spec_by_user++;
      video_adding_property.value = true;
    }

    function addVideoProperty2() {
      console.log('Add video property clicked');
      video.value[video_property.value] = script_store.video_spec[video_property.value].default;
      video_property.value = null;
      video_adding_property.value = false;
    }

    watch(
      () => script_store.request_video_spec_received,
      () => {
        console.log('Video spec received', script_store.video_spec);
      }
    );

    watch(
      () => script_store.request_video_spec_received,
      () => {
        console.log('Video spec received', script_store.video_spec);
      }
    );

    const video_properties = computed(() => {
      let existing_properties = Object.keys(video.value);
      return Object.keys(script_store.video_spec || {})
        .filter((prop) => !existing_properties.includes(prop))
        .sort();
    });

    const video_property = ref<string | null>(null);
    let video_adding_property = ref(false);

    return {
      parsed,
      gradients,
      objects,
      video,
      columns,
      selected,
      update_selected,
      save_changes,
      format_cell_value,
      update_cell_value,
      addObject,
      addVideoProperty,
      addVideoProperty2,
      video_adding_property,
      video_properties,
      video_property,
      scenes,
      addScene,
      removeScene,
    };
  },
});
</script>
