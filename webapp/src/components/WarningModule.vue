<template>
  <div></div>
</template>

<script lang="ts">
import { defineComponent, onMounted, onUnmounted } from 'vue';
import { useStore } from 'vuex';
import { ElNotification } from 'element-plus';
import { fetchAlarms } from '@/services/api'; // API 获取方法
import { delay } from '@/utils/delay'; // 延迟函数

export default defineComponent({
  name: 'WarningModule',
  setup() {
    const store = useStore(); // 使用 Vuex store
    let fetchInterval: number | null = null; // 用于存储定时器的 ID

    const fetchData = async () => {
      try {
        const response = await fetchAlarms();
        const alarms = response.data;

        // 提取和存储阈值信息
        const thresholds = alarms.threshold;
        store.commit('setThresholds', thresholds); // 存储阈值到 store

        // 显示报警信息
        const alarmMessages = alarms.message;
        if (Object.keys(alarmMessages).length > 0) { // 只有当有报警信息时才显示提示框
          for (const [ip, message] of Object.entries(alarmMessages)) {
            ElNotification({
              title: ip,
              message: String(message),
              type: 'error',
              duration: 8000, // 不自动关闭
              offset: 40, // 根据索引设置偏移量
            });

            await delay(500); // 每个通知之间延迟 500 毫秒
          }
        }
      } catch (error) {
        console.error('Failed to fetch Alarm Messages:', error);
      }
    };

    onMounted(() => {
      fetchData(); // 初始加载数据
      fetchInterval = window.setInterval(fetchData, 10000); // 每 10 秒调用一次 fetchData
    });

    onUnmounted(() => {
      if (fetchInterval !== null) {
        clearInterval(fetchInterval); // 组件卸载时清除定时器
      }
    });

    return {};
  }
});
</script>

<style scoped>
/* 你可以在这里添加样式 */
</style>
