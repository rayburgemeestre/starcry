<template>
    <div>
        <div class="columns toolbar">
            <b-navbar>
                <template slot="brand">
                    <b-navbar-item>
                        <img src="/sc.png" alt="STARCRY" style="max-height: 40px;">
                    </b-navbar-item>
                </template>
                <template slot="start">
                    <b-navbar-dropdown href="#" label="File">
                        <b-navbar-item href="#">Open</b-navbar-item>
                        <b-navbar-item href="#">Save</b-navbar-item>
                        <b-navbar-item href="#">Save as</b-navbar-item>
                        <b-navbar-item href="#">Exit</b-navbar-item>
                    </b-navbar-dropdown>
                    <b-navbar-dropdown href="#" label="Render">
                        <b-navbar-item href="#">Frame</b-navbar-item>
                        <b-navbar-item href="#">Video</b-navbar-item>
                    </b-navbar-dropdown>
                    <b-navbar-dropdown label="Help">
                        <b-navbar-item href="#">
                            About
                        </b-navbar-item>
                    </b-navbar-dropdown>
                </template>
            </b-navbar>
        </div>
        <div class="columns main-columns">
            <div class="column is-narrow">
                <b-menu>
                    <b-menu-list label="Menu">
                        <b-menu-item icon="information-outline" label="Files" :active="menu == 'files'" v-on:click="menu = menu == 'files' ? '' : 'files'"></b-menu-item>
                        <b-menu-item icon="cash-multiple" label="Script" :active="menu == 'script'" v-on:click="menu = menu == 'script' ? '' : 'script'"></b-menu-item>
                    </b-menu-list>
                    <!--
                    <b-menu-list label="Actions">
                        <b-menu-item label="Logout"></b-menu-item>
                    </b-menu-list>
                    -->
                </b-menu>
            </div>
            <div v-if="menu == 'files'" class="column" style="background-color: #c0c0c0; width: 38%; height: calc(100vh - 120px); overflow: scroll;">
                <scripts-component width="100%" height="100vh - 60px"/>
            </div>
            <div v-if="menu == 'script'" class="column" style="background-color: #c0c0c0; width: 38%;">
                <editor-component v-model="cpp_code" name="js" language="javascript" width="100%" height="100vh - 60px"/>
            </div>
            <div class="column" style="background-color: black; max-height: calc(100vh - 120px);">
                <canvas id="canvas" oncontextmenu="event.preventDefault()" style="width: 100%; max-height: calc(100vh - 120px);"></canvas>
            </div>
            <div class="column is-narrow">
                Third column
                <h2>{{ websock_status }}</h2>
                <h2>{{ websock_status2 }}</h2>
                <button v-shortkey="['ctrl', 's']" @shortkey="menu = menu == 'script' ? '' : 'script'">_</button>
                <button v-shortkey="['ctrl', 'f']" @shortkey="menu = menu == 'files' ? '' : 'files'">_</button>
                <button v-shortkey="[',']" @shortkey="prev_frame()">_</button>
                <button v-shortkey="['.']" @shortkey="next_frame()">_</button>
                <button v-shortkey="['ctrl', 'o']" @shortkey="get_objects()">_</button>
                <hr>
                <stats-component />
            </div>
        </div>
        <div class="columns" style="margin: 0px 20px">
            <playback-component v-bind:value="current_frame" />
        </div>
    </div>
</template>

<script>
    import EditorComponent from './components/EditorComponent.vue'
    import ScriptsComponent from './components/ScriptsComponent.vue'
    import PlaybackComponent from './components/PlaybackComponent.vue'
    import StatsComponent from './components/StatsComponent.vue'
    import StarcryAPI from './util/StarcryAPI'

    export default {
        data() {
            return {
                cpp_code: 'Hello world',
                websock_status: '',
                websock_status2: '',
                menu: 'files',
                filename: 'input/motion4.js',
                current_frame : 0,
                rendering: 0,
                max_queued: 10,
                _play: false,
            };
        },
        components: {
            EditorComponent,
            ScriptsComponent,
            PlaybackComponent,
            StatsComponent,
        },
        methods: {
            open: function(filename) {
                this.$data.filename = filename;
                this.script_endpoint.send("open " + filename);
                this.bitmap_endpoint.send(filename + " 0");
            },
            process_queue: function () {
                this._schedule_frames();
            },
            // playback
            play: function () {
                this.$data._play = true;
                this._schedule_frames();
            },
            stop: function () {
                this.$data._play = false;
            },
            prev_frame: function () {
                this.$data.current_frame--;
                if (!this.$data._play) {
                    this._schedule_frame();
                }
            },
            next_frame: function () {
                this.$data.current_frame++;
                if (!this.$data._play) {
                    this._schedule_frame();
                }
            },
            set_frame: function (frame) {
                this.$data.current_frame = frame;
                if (!this.$data._play) {
                    this._schedule_frame();
                }
            },
            _schedule_frames: function () {
                if (!this.$data._play) return;
                while (this.$data.rendering < this.$data.max_queued) {
                    this._schedule_frame();
                    this.$data.current_frame++;
                }
            },
            _schedule_frame: function () {
                this.$data.rendering++;
                this.bitmap_endpoint.send(this.$data.filename + " " + this.$data.current_frame);
            }
        },
        watch: {
            queued_frames(new_value) {
                this.process_queue();
            }
        },
        mounted: function() {
            this.bitmap_endpoint = new StarcryAPI(
                'bitmap',
                msg => {
                    this.$data.websock_status = msg;
                },
                buffer => {
                    Module.set_texture(buffer);
                    this.$data.rendering--;
                    this.process_queue();
                },
                _ => {}
            );
            this.script_endpoint = new StarcryAPI(
                'script',
                msg => {
                    this.$data.websock_status2 = msg;
                },
                buffer => {
                    let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
                    this.$data.cpp_code = str;
                },
                _ => {
                    this.script_endpoint.send("open " + this.$data.filename);
                }
            );
        }
    }

</script>

<style scoped>
    html, body {
        height: 100%;
        margin: 0;
    }

    .toolbar {
        margin: 0 !important;
    }
    .toolbar, .toolbar > * {
        background-color: #b1eedb;
    }

    .main-columns {
        min-height: calc(100vh - 60px*2);
        margin: 0;
        background-color: #f3f3f3;
    }

    canvas {
        cursor: none !important;
    }
    canvas:active {
        cursor: none !important;
    }
</style>
