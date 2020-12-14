export default class StarcryAPI {

    constructor(endpoint, on_status_change, on_message, on_connected) {
        this.endpoint = endpoint;
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
            message.data.arrayBuffer().then(this.on_message);
        }.bind(this);
        this.ws.onerror = function (error) {
            this.on_status_change("ERROR: " + error);
        }.bind(this);
    }

    send(msg) {
        this.ws.send(msg);
    }
}
