import Vue from 'vue'
import App from './App.vue'
import Buefy from 'buefy'
import 'buefy/dist/buefy.css'

Vue.use(Buefy);

if (!window.included) {
    window.included = true;
    // Vue.use(require('vue-shortkey'))
    new Vue({
        el: '#app',
        render: h => h(App)
    });
    require('./mystyles.scss');
}
