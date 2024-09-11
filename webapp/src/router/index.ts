import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router';
import ChartPage from '@/pages/ChartPage.vue';
import TablePage from '@/pages/TablePage.vue';

const routes: Array<RouteRecordRaw> = [
  {
    path: '/',
    redirect:'/chart',
    name: 'Home', 
  },
  {
    path:'/chart',
    name:'Charts', 
    component: ChartPage
  },
  {
    path: '/table',
    name: 'Table',
    component: TablePage
  }
];

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL || '/'),
  routes,
});

export default router;
