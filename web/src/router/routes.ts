import { RouteRecordRaw } from 'vue-router';

const routes: RouteRecordRaw[] = [
  {
    path: '/',
    redirect: '/editor',
  },
  {
    path: '/files',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/FilesPage.vue') }],
  },
  {
    path: '/script',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/ScriptPage.vue') }],
  },
  {
    path: '/editor',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/EditorPage.vue') }],
  },
  {
    path: '/objects',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/ObjectsPage.vue') }],
  },
  {
    path: '/help',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/HelpPage.vue') }],
  },
  {
    path: '/debug',
    component: () => import('layouts/MainLayout.vue'),
    children: [{ path: '', component: () => import('pages/DebugPage.vue') }],
  },

  // Always leave this as last one,
  // but you can also remove it
  {
    path: '/:catchAll(.*)*',
    component: () => import('pages/ErrorNotFound.vue'),
  },
];

export default routes;
