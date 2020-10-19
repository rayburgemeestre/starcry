import Vue from 'vue'
import App from './App.vue'
import Buefy from 'buefy'
import 'buefy/dist/buefy.css'

Vue.use(Buefy);
Vue.use(require('vue-shortkey'));

if (!window.included) {
    window.included = true;
    new Vue({
        el: '#app',
        render: h => h(App)
    });
    require('./mystyles.scss');
}

function uuidv4() {
    return ([1e7]+-1e3+-4e3+-8e3+-1e11).replace(/[018]/g, c =>
        (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
    );
}
console.log(uuidv4());
