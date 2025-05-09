import { StarcryAPI } from 'components/api';
import { JsonWithObjectsParser } from 'components/json_parser';
import { create_tree, sort_tree } from 'components/filetree';
import { useScriptStore } from 'stores/script';
import { useFilesStore } from 'stores/files';
import { useViewpointStore } from 'stores/viewpoint';
import { useProjectStore } from 'stores/project';
import { watch } from 'vue';
import { hash } from 'components/hash';

let timeout_script_updates: NodeJS.Timeout | null = null;

export function create_script_endpoint() {
  const script_store = useScriptStore();
  const project_store = useProjectStore();
  const files_store = useFilesStore();
  const viewpoint_store = useViewpointStore();

  const self = new StarcryAPI(
    'script',
    StarcryAPI.text_type,
    (msg: string) => {
      console.log(msg);
    },
    (buffer: string) => {
      if (buffer[0] === '1') {
        if (script_store.filename !== '') {
          console.log('not opening file ' + buffer.slice(1) + ' because ' + script_store.filename + ' is already open');
        } else {
          script_store.filename = buffer.slice(1);
        }
        self.send('open ' + script_store.filename);
        script_store.render_requested_by_user++;
      } else if (buffer[0] === '2') {
        script_store.script = buffer.slice(1);

        project_store.parser = new JsonWithObjectsParser(script_store.script, window.sc_constants);
        // this.$data.input_source = buffer;
        script_store.video = project_store.parser.parsed()['video'] || {};
        script_store.preview = project_store.parser.parsed()['preview'] || {};
        viewpoint_store.scale = script_store.video['scale'] || 1;
        let total_duration = 0;
        script_store.frames_per_scene = [];
        for (const scene of project_store.parser.parsed()['scenes']) {
          if (scene.duration) {
            total_duration += scene.duration;
          }
          script_store.frames_per_scene.push(scene.duration * script_store.video['fps']);
        }
        if (!total_duration) total_duration = script_store.video['duration'];
        script_store.max_frames = Math.floor(total_duration * script_store.video['fps']);
        // the editorpane needs to be updated
        script_store.re_render_editor_sidepane++;
      } else if (buffer[0] === '3') {
        script_store.filename = buffer.slice(1);
      } else if (buffer[0] === '4') {
        // traverse tree and sort all children
        const tree = create_tree(buffer.slice(1));
        sort_tree(tree);
        files_store.simple = tree;
      } else if (buffer[0] === '5') {
        console.log('server terminated.');
      } else if (buffer[0] === '6') {
        window.sc_constants = buffer.slice(1);
        eval(window.sc_constants);
      } else if (buffer[0] === '7') {
        script_store.video_spec = JSON.parse(buffer.slice(1));
        script_store.request_video_spec_received++;
      } else if (buffer[0] === '8') {
        script_store.object_spec = JSON.parse(buffer.slice(1));
        script_store.request_object_spec_received++;
      }
    },
    (_) => {
      self.send('list');
    }
  );

  // user has made changes to the project javascript through the editor
  watch(
    () => script_store.value_updated_by_user,
    () => {
      if (timeout_script_updates) clearTimeout(timeout_script_updates);
      timeout_script_updates = setTimeout(
        function () {
          self.send('set ' + script_store.script);
          viewpoint_store.script_hash = '' + hash(script_store.script);
          // doesn't work, because the viewpoint serialization is not sufficient
          script_store.render_requested_by_user++;
        }.bind(this),
        1000
      );
    }
  );

  // different script has been selected in the tree
  watch(
    () => files_store.selected,
    () => {
      self.send('open ' + files_store.selected);
      script_store.filename = files_store.selected;
    }
  );

  watch(
    () => script_store.restart_server_requested_by_user,
    () => {
      // trust kubernetes to restart the pod
      self.send('terminate');
    }
  );

  watch(
    () => script_store.request_video_spec_by_user,
    () => {
      self.send('get_video_spec');
    }
  );

  watch(
    () => script_store.request_object_spec_by_user,
    () => {
      self.send('get_object_spec');
    }
  );

  return self;
}
