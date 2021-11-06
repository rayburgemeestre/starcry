import Buefy from 'buefy'
import 'buefy/dist/buefy.css'

import './mystyles.scss'

import Vue from 'vue'
import App from './App.vue'

Vue.use(Buefy);
Vue.use(require('vue-shortkey'));

if (!window.included) {
    window.included = true;
    new Vue({
        el: '#app',
        render: h => h(App)
    });
}

function uuidv4() {
    return ([1e7]+-1e3+-4e3+-8e3+-1e11).replace(/[018]/g, c =>
        (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
    );
}

function load_client_data() {
    return localStorage.getItem('client-data') ? JSON.parse(localStorage.getItem('client-data')) : {
        'ID': uuidv4()
    };
}

let client_data = load_client_data();

function save_client_data(client_data) {
    localStorage.setItem('client-data', JSON.stringify(client_data));
}

save_client_data(client_data);