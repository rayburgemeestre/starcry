import { StarcryAPI } from 'components/api';
import { useStatsStore } from 'stores/stats';
import { useScriptStore } from 'stores/script';

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
        const jobs = buffer.data.jobs || [];
        for (const job of jobs) {
          if (job.state == 'SKIPPED') {
            skipped = Math.max(skipped, job.number);
          } else if (job.state == 'RENDERING') {
            rendering = Math.max(rendering, job.number);
          } else if (job.state == 'RENDERED') {
            rendered = Math.max(rendered, job.number);
          } else if (job.state == 'QUEUED') {
            queued = Math.max(queued, job.number);
          }
        }
        script_store.job_skipped = skipped;
        script_store.job_rendering = rendering;
        script_store.job_rendered = rendered;
        script_store.job_queued = queued;
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
