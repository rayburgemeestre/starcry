<template>
    <div style="width: 100%">
        <div v-for="dat in statistics">
            <b>{{ dat.name }}</b> {{ dat.counter }} <br/>
        </div>
    </div>
</template>

<script>
    import StarcryAPI from '../util/StarcryAPI'
    export default {
        data() {
            return {
                statistics: []
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
                    this.$data.statistics = buffer;
                },
                _ => {}
            );
        }
    }
</script>

<style scoped>
</style>
