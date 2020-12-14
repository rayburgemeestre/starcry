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
                _ => {},
                buffer => {
                    let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
                    this.$data.statistics = JSON.parse(str);
                },
                _ => {}
            );
        }
    }
</script>

<style scoped>
</style>
