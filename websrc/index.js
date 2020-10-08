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
