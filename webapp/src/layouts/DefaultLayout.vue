<!-- src/layouts/DefaultLayout.vue -->
<template>
  <el-container class="layout-container" direction="vertical">
    <el-header height="55px" class="el-header">
      <HeaderComponent />
    </el-header>
    <el-container class="foot-container" direction="horizontal">
      <el-scrollbar class="sidebar">
          <MenuComponent />
      </el-scrollbar>
      <el-main class="main">      
          <RouterView />
      </el-main>   
    </el-container>
  </el-container>
</template>

<script setup lang="ts">
import MenuComponent from '@/components/Menu.vue';
import HeaderComponent from '@/components/Header.vue';
import AlertBox from '@/components/AlertBox.vue';
</script>

<style scoped>
.layout-container {
  height: 100%;
  width: 100%;
  overflow: hidden;
}

.el-header {
  display: flex;
  border-bottom: 1px solid #dcdfe6;
}

.sidebar {
  display: flex;
  transform: translate(0);
  width: 200px;
  overflow-y: auto;
  border-right: 1px solid #dcdfe6;
}

.main {
  padding: 0;
}
</style>
