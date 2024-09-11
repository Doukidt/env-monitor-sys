<!-- src/components/Menu.vue -->
<template>
  <el-aside class="menu" width="200px">
    <el-menu default-active="2" class="el-menu-vertical">
      <el-menu-item
        v-for="item in getMenu()"
        :key="item.path"
        @click="clickMenu(item)"
      >
        <component class="icons" :is="item.icon"></component>
        <span>{{ item.name }}</span>
      </el-menu-item>
    </el-menu>
  </el-aside>
</template>

<script setup lang="ts">
import { useRouter } from 'vue-router';
import { useStore } from 'vuex';

const router = useRouter();
const store = useStore();

const getMenu = () => {
  return store.state.menu;
}

const clickMenu = (key : any) => {
  console.log(key);
  store.dispatch('updateSelectedDeviceIp', 'Select An IP');
  router.push({ path: key.path });
};
</script>

<style scoped>
.el-menu-vertical {
  border-right: 0;
}

.icons {
  width: 18px;
  height: 18px;
  padding-right: 4px;
}
</style>
