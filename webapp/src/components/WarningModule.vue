<template>
  <div></div>
</template>

<script lang="ts">
import { defineComponent, onMounted } from 'vue';
import { ElNotification } from 'element-plus';
import { fetchAlarms } from '@/services/api';
import { delay } from '@/utils/delay';

export default defineComponent({
  name: 'WarningModule',
  setup() {
    const fetchData = async () => {
      try {
        const response = await fetchAlarms();
        const alarms = response.data;

        for (const [index, ip] of Object.keys(alarms).entries()) {
          const message = alarms[ip];
          ElNotification({
            title: ip,
            message: message,
            type: 'error',
            duration: 0, // 不自动关闭
            showClose: false, // 隐藏关闭按钮
            offset: 40, // 根据索引设置偏移量
          });

          await delay(500); // 每个通知之间延迟 500 毫秒
        }
      } catch (error) {
        console.error('Failed to catch Alarm Message:', error);
      }
    };

    onMounted(() => {
      fetchData();
    });

    return {};
  }
});
</script>

<style scoped>
/* 你可以在这里添加样式 */
</style>
