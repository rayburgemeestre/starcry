import { StarcryAPI } from 'src/core/api';
import { useStatsStore } from 'stores/stats';
import { useScriptStore } from 'stores/script';
import { useBitmapStore } from 'stores/bitmap';

export function create_stats_endpoint() {
  const stats_store = useStatsStore();
  const script_store = useScriptStore();

  const self = new StarcryAPI(
    'stats',
    StarcryAPI.json_type,
    (msg: string) => {
      // status.value = msg;
    },
    (buffer: string) => {
      // msgs.value[buffer.type] = buffer;
      if (buffer.type === 'metrics') {
        stats_store.rows = [];
        const threads = buffer.data.threads || [];
        for (const thread of threads) {
          stats_store.rows.push([thread.name, Math.round(thread.seconds_idle), thread.state]);
        }

        let skipped = -1;
        let rendering = -1;
        let rendered = -1;
        let queued = -1;
        let timedout = -1;
        const jobs = buffer.data.jobs || [];
        let last_job = null;
        for (const job of jobs) {
          last_job = job;
          if (job.state == 'SKIPPED') {
            skipped = Math.max(skipped, job.number);
          } else if (job.state == 'RENDERING') {
            rendering = Math.max(rendering, job.number);
          } else if (job.state == 'RENDERED') {
            rendered = Math.max(rendered, job.number);
          } else if (job.state == 'QUEUED') {
            queued = Math.max(queued, job.number);
          } else if (job.state == 'TIMEOUT') {
            timedout = Math.max(timedout, job.number);
            // since there is no bitmap going to be arriving in the case of a timeout
            const bitmap_store = useBitmapStore();
            bitmap_store.loading = false;
          }
        }
        if (last_job) {
          stats_store.render_status = last_job.state;
          stats_store.render_label = last_job.state.indexOf('RENDER') !== -1 ? 'Time' : 'Frame';
          stats_store.render_value = last_job.render_time || last_job.number;
        }
        script_store.job_skipped = skipped;
        script_store.job_rendering = rendering;
        script_store.job_rendered = rendered;
        script_store.job_queued = queued;
        script_store.job_timedout = timedout;
      } else if (buffer.type === 'stats') {
        stats_store.rows_piper.value = [];
        const components = buffer.data || [];
        for (const component of components) {
          if (component.counter) stats_store.rows_piper.value.push([component.name, component.counter]);
        }
      } else if (buffer.type == 'fs_change') {
        script_store.script = buffer['source'];
      }
    },
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    (_) => {}
  );
  return self;
}
