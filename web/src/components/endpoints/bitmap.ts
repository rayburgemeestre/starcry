import { StarcryAPI } from 'components/api';
import { useScriptStore } from 'stores/script';
import { useBitmapStore } from 'stores/bitmap';
import { watch } from 'vue';

export function create_bitmap_endpoint() {
  const script_store = useScriptStore();
  const bitmap_store = useBitmapStore();

  const self = new StarcryAPI(
    'bitmap',
    StarcryAPI.binary_type,
    (msg) => {
      // this.$data.websock_status = msg;
    },
    (buffer) => {
      window.Module.last_buffer = buffer;
      window.Module.set_texture(buffer);
      // this.$data.rendering--;
      // this.process_queue();
      bitmap_store.loading = false;
      // allow other listeners to update the canvas2 labels
      script_store.render_completed_by_server++;
    },
    (_) => {
      // this.log('DEBUG', 'bitmap', 'websocket connected', '');
      // this.$data.connected_bitmap = true;
    },
    (_) => {
      // this.log('DEBUG', 'bitmap', 'websocket disconnected', '');
      // this.$data.connected_bitmap = false;
    }
  );

  watch(
    () => bitmap_store.outbox.length,
    function () {
      for (const msg of bitmap_store.outbox) {
        self.send(JSON.stringify(msg));
        bitmap_store.outbox = [];
      }
    }
  );

  return self;
}
