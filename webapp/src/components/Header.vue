<!-- src/components/Header.vue -->
<template>
  <div class="header-container">
    <div class="title">
      <h1>Env Monitor Sys</h1>
    </div>
    <!-- 使用 Element Plus 下拉菜单 -->
    <el-dropdown @command="handleDeviceChange">
      <span class="el-dropdown-link">
        {{ selectedDevice }}
        <el-icon class="el-icon--right">
          <arrow-down />
        </el-icon>
      </span>
      <template #dropdown>
        <el-dropdown-menu>
          <el-dropdown-item
            v-for="(device, index) in devices"
            :key="index"
            :command="device.ip"
          >
            {{ device.ip }}
          </el-dropdown-item>
        </el-dropdown-menu>
      </template>
    </el-dropdown>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue';
import { useStore } from 'vuex'; // 导入 Vuex
import { ElDropdown, ElDropdownMenu, ElDropdownItem, ElIcon } from 'element-plus';
import { ArrowDown } from '@element-plus/icons-vue';
import { fetchExistIP } from '@/services/api'; // 导入 fetchExistIP 函数

const store = useStore(); // 使用 Vuex store

// 从 store 中获取选中的设备 IP
const selectedDevice = computed(() => store.getters.selectedDeviceIp);

// 用于存储设备数据的状态
const devices = ref<{ ip: string; name: string }[]>([]);

// 处理设备选择
function handleDeviceChange(command: string) {
  store.dispatch('updateSelectedDeviceIp', command); // 更新选中的设备 IP 到 store
}

// 获取设备 IP 数据
async function loadDevices() {
  try {
    const ipData = await fetchExistIP(); // 调用 API 函数获取数据
    
    devices.value = ipData.data.map((ip: string, index: number) => ({ ip, name: `Device ${index + 1}` }));
  } catch (error) {
    console.error('Error loading devices:', error);
  }
}

// 在组件挂载时加载设备数据
onMounted(loadDevices);
</script>

<style scoped>
.header-container {
  flex:1;
  display: flex;
  align-items: center;
  justify-content: space-between; /* 标题和下拉菜单分布在两边 */
  padding: 16px;
}

.title {
  display: flex;
  align-items: center;
}

.el-dropdown {
  cursor: pointer;
  color: var(--el-color-primary);
  display: flex;
  align-items: center;  
}

/* 禁用下拉菜单链接的默认边框样式 */
.el-dropdown-link:focus,
.el-dropdown-link:hover {
  outline: none; /* 禁用聚焦时的外框 */
  box-shadow: none; /* 禁用聚焦时的阴影 */
}

/* 禁用下拉菜单项的默认悬停和聚焦样式 */
.el-dropdown-menu .el-dropdown-item:focus,
.el-dropdown-menu .el-dropdown-item:hover {
  background-color: transparent; /* 禁用悬停时的背景颜色 */
  outline: none; /* 禁用聚焦时的外框 */
  box-shadow: none; /* 禁用聚焦时的阴影 */
}

h1 {
  font-size: large;
}
</style>
