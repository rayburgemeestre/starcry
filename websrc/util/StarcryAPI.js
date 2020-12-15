export default class StarcryAPI {
    static get binary_type() { return 1; }
    static get json_type() { return 2; }
    static get text_type() { return 3; }

    constructor(endpoint, type, on_status_change, on_message, on_connected) {
        this.endpoint = endpoint;
        this.type = type;
        this.on_status_change = on_status_change;
        this.on_message = on_message;
        this.on_connected = on_connected;
        this.ws = false;
        this.retry = false;
        this.connect();
    }

    connect() {
        this.on_status_change('connecting');
        let protocol = document.location.protocol.replace('http', 'ws');
        if (document.location.href.indexOf('localhost')) {
            this.ws = new WebSocket(protocol + '//' + document.location.host.replace(':8080', ':18080') + '/' + this.endpoint, ['tag_test']);
        } else {
            this.ws = new WebSocket(protocol + '//' + document.location.host + '/' + this.endpoint, ['tag_test']);
        }
        this.ws.onopen = function () {
            clearTimeout(this.retry);
            this.on_status_change('connected');
            this.on_connected();
        }.bind(this);
        this.ws.onclose = function () {
            this.on_status_change('disconnected');
            this.retry = setTimeout(this.connect, 1000);
        }.bind(this);
        this.ws.onmessage = function (message) {
            switch (this.type) {
                case StarcryAPI.binary_type:
                    message.data.arrayBuffer().then(this.on_message);
                    break;
                case StarcryAPI.json_type:
                    console.log(this.endpoint);
                    console.log(message.data);
                    this.on_message(JSON.parse(message.data));
                    break;
                case StarcryAPI.text_type:
                    this.on_message(message.data);
                    break;
            }
        }.bind(this);
        this.ws.onerror = function (error) {
            this.on_status_change("ERROR: " + error);
        }.bind(this);
    }

    send(msg) {
        this.ws.send(msg);
    }
}
