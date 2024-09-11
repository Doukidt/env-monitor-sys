<!-- src/App.vue -->
<template>
  <DefaultLayout />
  <WarningModule />
</template>

<script setup lang="ts">
import DefaultLayout from './layouts/DefaultLayout.vue';
import WarningModule from './components/WarningModule.vue';
</script>

<style>
#app {
  height: 100%;
  width: 100%;
  background-color: rgb(255, 255, 255);
}
</style>