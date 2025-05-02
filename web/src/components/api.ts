import { load_client_data, save_client_data } from 'components/clientstorage';
import { useGlobalStore } from 'stores/global';

export interface StarcryAPI {
  endpoint: string;
  type: number;
  on_status_change: any;
  on_message: any;
  on_connected: any;
  on_disconnected: any;
  ws: WebSocket | boolean;
  retry: boolean;
  client_data: string;
}

// TODO: further port everything to typescript, and do it in a better way
export class StarcryAPI {
  static get binary_type() {
    return 1;
  }
  static get json_type() {
    return 2;
  }
  static get text_type() {
    return 3;
  }

  constructor(
    endpoint: string,
    type: number,
    on_status_change?: any,
    on_message?: any,
    on_connected?: any,
    on_disconnected?: any
  ) {
    this.endpoint = endpoint;
    this.type = type;
    this.on_status_change = on_status_change;
    this.on_message = on_message;
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    this.on_connected = on_connected || function () {};
    // eslint-disable-next-line @typescript-eslint/no-empty-function
    this.on_disconnected = on_disconnected || function () {};
    this.ws = false;
    this.retry = false;

    save_client_data(load_client_data());

    if (!localStorage.getItem('client-data')) {
      throw 'client-data could not be read from local storage.';
    }

    this.client_data = JSON.parse(<string>localStorage.getItem('client-data'));
    this.connect();
  }

  connect() {
    this.on_status_change('connecting');
    const protocol: string = document.location.protocol.replace('http', 'ws');
    if (document.location.href.indexOf('localhost')) {
      this.ws = new WebSocket(
        protocol +
          '//' +
          document.location.host.replace(':8080', ':18080').replace(':9000', ':18080') +
          '/' +
          this.endpoint,
        [this.client_data['ID']]
      );
    } else {
      this.ws = new WebSocket(protocol + '//' + document.location.host + '/' + this.endpoint, [this.client_data['ID']]);
    }
    this.ws.onopen = function () {
      clearTimeout(this.retry);
      this.on_status_change('connected');
      useGlobalStore().connected.add(this.endpoint);
      useGlobalStore().disconnected.delete(this.endpoint);
      this.send('LINK ' + this.client_data['ID']);
      this.on_connected();
    }.bind(this);
    this.ws.onclose = function () {
      this.on_status_change('disconnected');
      this.on_disconnected();
      useGlobalStore().connected.delete(this.endpoint);
      useGlobalStore().disconnected.add(this.endpoint);
      this.retry = setTimeout(this.connect.bind(this), 100);
    }.bind(this);
    this.ws.onmessage = function (message) {
      switch (this.type) {
        case StarcryAPI.binary_type:
          message.data.arrayBuffer().then(
            function (buffer) {
              try {
                this.on_message(buffer);
              } catch (error) {
                console.error('Error calling on_message:', error);
              }
            }.bind(this)
          );
          break;
        case StarcryAPI.json_type:
          this.on_message(JSON.parse(message.data));
          break;
        case StarcryAPI.text_type:
          this.on_message(message.data);
          break;
      }
    }.bind(this);
    this.ws.onerror = function (error) {
      this.on_status_change('ERROR: ' + error);
    }.bind(this);
  }

  send(msg: string) {
    this.ws.send(msg);
  }
}
