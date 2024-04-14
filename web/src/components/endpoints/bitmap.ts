import { StarcryAPI } from 'components/api';
import { useScriptStore } from 'stores/script';
import { useBitmapStore } from 'stores/bitmap';
import { watch } from 'vue';
import { useStatsStore } from 'stores/stats';

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
      if (buffer.byteLength > 0) {
        window.Module.last_buffer = buffer;
        window.Module.set_texture(buffer);
      }
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

  const stats_store = useStatsStore();
  watch(
    () => bitmap_store.outbox.length,
    function () {
      stats_store.render_status = '';
      stats_store.render_label = 'please wait';
      stats_store.render_value = 'V8 is busy';
      for (const msg of bitmap_store.outbox) {
        self.send(JSON.stringify(msg));
        bitmap_store.outbox = [];
      }
    }
  );

  return self;
}
