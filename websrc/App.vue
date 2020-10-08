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
                <h1>{{ ticks }}</h1>
                <h2>{{ websock_status }}</h2>
                <button v-shortkey="['ctrl', 's']" @shortkey="menu = menu == 'script' ? '' : 'script'">_</button>
                <button v-shortkey="['ctrl', 'f']" @shortkey="menu = menu == 'files' ? '' : 'files'">_</button>
            </div>
        </div>
        <div class="columns" style="margin: 0px 20px">
            <b-slider :min="0" :max="250" v-model="ticks" ticks></b-slider>
        </div>
    </div>
</template>

<script>
    import EditorComponent from './components/EditorComponent.vue'
    import ScriptsComponent from './components/ScriptsComponent.vue'
    let ws;
    let ws_script;
    let retry;
    let retry2;
    export default {
        data() {
            return {
                cpp_code: 'Hello world',
                ticks: 0,
                websock_status: '',
                menu: 'files',
            };
        },
        components: {
            EditorComponent,
            ScriptsComponent
        },
        methods: {
            connect: function() {
                this.$data.websock_status = 'connecting';
                let protocol = document.location.protocol.replace('http', 'ws');
                if (document.location.href.indexOf('localhost')) {
                    ws = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/bitmap', ['tag_test']);
                    ws_script = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/script', ['tag_test']);
                    console.log(document.location.host.replace(':8080', ':18080'));
                } else {
                    ws = new WebSocket(protocol + '//' + document.location.host + '/bitmap', ['tag_test']);
                    ws_script = new WebSocket(protocol + '//' + document.location.host + '/script', ['tag_test']);
                }
                ws.onopen = function () {
                    clearTimeout(retry);
                    this.$data.websock_status = 'connected';
                }.bind(this);
                ws.onclose = function () {
                    this.$data.websock_status = 'disconnected';
                    retry = setTimeout(this.connect, 1000);
                }.bind(this);
                ws_script.onopen = function () {
                    clearTimeout(retry2);
                    this.$data.websock_status = 'connected';
                    ws_script.send("open input/test.js");
                }.bind(this);
                ws_script.onclose = function () {
                    this.$data.websock_status = 'disconnected';
                    retry2 = setTimeout(this.connect, 1000);
                }.bind(this);
                ws.onmessage = function (message) {
                    const reader = new FileReader();
                    reader.addEventListener('loadend', (e) => {
                        const text = e.srcElement.result;
                        console.log(text.length);
                        Module.set_texture(text);
                    });
                    reader.addEventListener('error', (e) => {
                        console.log(e);
                    });
                    //reader.readAsText(message.data);
                    // Module.set_texture(message.data);
                    message.data.arrayBuffer().then(buffer => {
                        Module.set_texture(buffer);
                    });

                };
                ws_script.onmessage = function (message) {
                    const reader = new FileReader();
                    reader.addEventListener('loadend', (e) => {
                        const text = e.srcElement.result;
                        console.log('this is one');
                        console.log(text);
                        console.log(text.length);
                        //Module.set_texture(text);
                    });
                    reader.addEventListener('error', (e) => {
                        console.log(e);
                    });
                    message.data.arrayBuffer().then(function(buffer) {
                        //Module.set_texture(buffer);
                        let str = String.fromCharCode.apply(null, new Uint8Array(buffer));
                        this.$data.cpp_code = str;
                        console.log(str);
                    }.bind(this));
                }.bind(this);
                ws.onerror = function (error) {
                    console.log("ERROR: " + error);
                };
            }
        },
        watch: {
            ticks: function(frame) {
                ws.send("input/test.js " + frame);
            },
        },
        mounted: function() {
            this.connect();
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
