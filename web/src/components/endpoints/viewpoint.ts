import { StarcryAPI } from 'components/api';
import { useViewpointStore } from 'stores/viewpoint';
import { useScriptStore } from 'stores/script';
import { watch } from 'vue';

export function serialize_viewpoint() {
  const viewpoint_store = useViewpointStore();
  return JSON.stringify({
    operation: 'set',
    ray: 1,
    scale: viewpoint_store.scale,
    offset_x: viewpoint_store.offset_x,
    offset_y: viewpoint_store.offset_y,
    raw: viewpoint_store.raw,
    preview: viewpoint_store.preview,
    labels: viewpoint_store.labels,
    caching: viewpoint_store.caching,
    debug: !!viewpoint_store.debug,
    save: viewpoint_store.save,
    canvas_w: viewpoint_store.canvas_w,
    canvas_h: viewpoint_store.canvas_h,
    script_hash: viewpoint_store.script_hash,
  });
}

export function create_viewpoint_endpoint() {
  const viewpoint_store = useViewpointStore();
  const script_store = useScriptStore();

  const self = new StarcryAPI(
    'viewpoint',
    StarcryAPI.json_type,
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    (msg) => {},
    (buffer) => {
      viewpoint_store.scale = buffer['scale'];
      // this.$data.view_x = buffer["offset_x"] / buffer["scale"];
      // this.$data.view_y = buffer["offset_y"] / buffer["scale"];
      // this.$data.offsetX = buffer["offset_x"];
      // this.$data.offsetY = buffer["offset_y"];
      viewpoint_store.raw = buffer['raw'];
      viewpoint_store.preview = buffer['preview'];
      viewpoint_store.save = buffer['save'];
      viewpoint_store.labels = buffer['labels'];
      viewpoint_store.caching = buffer['caching'];
      viewpoint_store.debug = buffer['debug'];
      script_store.texture_w = buffer['canvas_w'];
      script_store.texture_h = buffer['canvas_h'];

      if (serialize_viewpoint() != viewpoint_store.previous_hash) {
        script_store.render_requested_by_user_v2++;
      }
      viewpoint_store.previous_hash = serialize_viewpoint();
    },
    (_) => {
      if (viewpoint_store.first_load)
        self.send(
          JSON.stringify({
            operation: 'read',
          })
        );
      viewpoint_store.first_load = false;
    }
  );

  watch(
    () => viewpoint_store.viewpoint_send_needed,
    (n) => {
      if (!viewpoint_store.current_message) return;
      self.send(viewpoint_store.current_message);
    }
  );
  return self;
}
