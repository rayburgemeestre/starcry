<template>
    <div style="width: 100%">
        <div v-for="dat in metrics['threads']">
          <b-progress
              :value="100"
              size="is-medium"
              show-value
          >
            <span>{{ dat.name }}</span>
            <span v-if="dat.state == 'IDLE'" style="color: white">IDLE {{ Math.round(dat.seconds_idle) }}</span>
            <span v-if="dat.state == 'BUSY'" style="color: red">BUSY {{ Math.round(dat.job.seconds_busy) }}</span>
          </b-progress>
        </div>
      <div>
        <b-progress
            :value="job_progress"
            size="is-medium"
            show-value
        >
          jobs
        </b-progress>
      </div>
      <div>
        <b-progress
            v-if="busy_last_frame"
            :value="chunk_progress"
            size="is-medium"
            show-value
        >
          frame
        </b-progress>
      </div>
      <!--
      <div v-for="dat in statistics">
          <b>{{ dat.name }}</b> {{ dat.counter }} <br/>
      </div>
      -->
    </div>
</template>

<script>
import StarcryAPI from '../util/StarcryAPI'
export default {
  data() {
    return {
      statistics: [],
      metrics: []
    }
  },
  computed: {
    job_progress: function () {
      var m = 0;
      for (var job of this.$data.metrics.jobs) {
        m = Math.max(job.number, m);
      }
      return 100 * (m / this.$parent.current_frame);
    },
    busy_last_frame: function() {
      return this.$data.metrics.jobs.slice(-1)[0].number === this.$parent.current_frame;
    },
    chunk_progress: function () {
      let job = this.$data.metrics.jobs.slice(-1)[0];
      if (job.chunks.frame) {
        let total = job.chunks.frame.length;
        let rendered = 0;
        for (let frame of job.chunks.frame) {
          if (frame.state === 'RENDERED') {
            rendered++;
          }
        }
        return 100 * (rendered / total);
      }
      return 0;
    }
  },
  methods: {
  },
  watch: {
  },
  mounted: function() {
    this.stats_endpoint = new StarcryAPI(
        'stats',
        StarcryAPI.json_type,
        _ => {},
        buffer => {
          if (buffer['type'] === 'stats') {
            this.$data.statistics = buffer['data'];
          }
          else if (buffer['type'] === 'metrics') {
            this.$data.metrics = buffer['data'];
          }
          else if (buffer['type'] === 'fs_change') {
            console.log(buffer['file'], this.$parent.filename);
            if (buffer['file'] === this.$parent.filename) {
              this.$parent.set_frame(this.$parent.current_frame);
            }
          }
        },
        _ => {}
    );
  }
}
</script>

<style scoped>
</style>
