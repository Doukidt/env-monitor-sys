import { createApp } from 'vue';
import App from './App.vue';
import router from './router';
import store from './store';
import "./assets/css/reset.css";
import ElementPlus from 'element-plus';
import 'element-plus/dist/index.css';
// import './mock/index.ts';

const app = createApp(App);

app.use(router);
app.use(store);
app.use(ElementPlus);

app.mount('#app');
