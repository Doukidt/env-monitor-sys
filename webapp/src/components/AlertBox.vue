<!-- src/components/AlertBox.vue -->
<template>
  <div v-if="isVisible" class="alert-box">
    <el-alert
      title="警告信息"
      type="error"
      :description="message"
      show-icon
    />
  </div>
</template>

<script setup lang="ts">
import { ref, watchEffect } from 'vue';

// 警告信息的可见性和内容
const isVisible = ref(false);
const message = ref('');

// 模拟接收警告信息
watchEffect(() => {
  // 监听到警告信息，展示提示框
  isVisible.value = true;
  message.value = '收到警告信息！';
});
</script>

<style scoped>
.alert-box {
  position: absolute;
  top: 20px;
  right: 20px;
  z-index: 1000;
}
</style>
