<template>
  <div class="container">
    <div class="filters">
      <!-- Start Time 选择框 -->
      <div class="filter-item">
        <label class="filter-label">Start Time:</label>
        <el-date-picker
          v-model="startTime"
          type="datetime"
          placeholder="Select Start Time"
          class="inner-picker"
          @change="filterData"
        />
      </div>
      <!-- End Time 选择框 -->
      <div class="filter-item">
        <label class="filter-label">End Time:</label>
        <el-date-picker
          v-model="endTime"
          type="datetime"
          placeholder="Select End Time"
          class="inner-picker"
          @change="filterData"
        />
      </div>
    </div>
    <el-table :data="paginatedData" class="table">
      <el-table-column prop="eid" label="EID" />
      <el-table-column prop="etime" label="Time" />
      <!-- 动态生成列，并加粗显示所有以 'Val' 结尾的数据 -->
      <el-table-column v-for="col in dynamicColumns" :key="col.prop" :prop="col.prop" :label="col.label">
        <template #default="{ row }">
          <span :style="{ fontWeight: col.bold ? 'bold' : 'normal' }">{{ row[col.prop] }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="note" label="Notes" />
    </el-table>
    <div class="pagination">
      <el-pagination
        background
        layout="prev, pager, next"
        :total="totalItems"
        :page-size="pageSize"
        :current-page="currentPage"
        @current-change="handlePageChange"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, watch, computed } from 'vue';
import { fetchRealTimeData } from '../services/api';
import { useStore } from 'vuex';

// 定义列的类型
type ColumnType = {
  prop: string;
  label: string;
  bold?: boolean;
};

// 定义表格行的类型
type TableRow = {
  eid: number;
  etime: string;
  ip: string;
  [key: string]: any; // 允许动态属性
};

// 状态管理
const startTime = ref<Date | null>(null);
const endTime = ref<Date | null>(null);
const tableData = ref<TableRow[]>([]);
const filteredData = ref<TableRow[]>([]);
const totalItems = ref(0);
const dynamicColumns = ref<ColumnType[]>([]);
const pageSize = ref(10); // 每页显示的行数
const currentPage = ref(1); // 当前页码
const store = useStore();

let intervalId: ReturnType<typeof setInterval>;
let lastEid: string | null = null; // 存储上一次获取的数据的eid

// 计算分页数据
const paginatedData = computed(() => {
  const start = (currentPage.value - 1) * pageSize.value;
  const end = start + pageSize.value;
  return filteredData.value.slice(start, end);
});

// 处理分页变更
const handlePageChange = (page: number) => {
  currentPage.value = page;
};

// 动态生成列
const generateDynamicColumns = (data: any) => {
  const columns: ColumnType[] = [];
  for (const key in data) {
    if (/.+Val$/.test(key)) {  // 匹配以 'Val' 结尾的键
      columns.push({
        prop: key,
        label: key.replace(/Val$/, ''), // 去掉 'Val' 后缀作为标签名
        bold: true,  // 设置为加粗
      });
    }
  }
  dynamicColumns.value = columns;
};

// 模拟获取数据
const fetchData = async () => {
  const ip = store.getters.selectedDeviceIp;
  if (ip !== 'Select An IP') {
    const data = await fetchRealTimeData(ip);
    if (data) {
      console.log('Fetched Data:', data);  // 打印获取的数据
      // 检查新获取的数据的eid是否与上一次相同
      if (data.eid === lastEid) {
        console.log('数据重复，跳过插入');
        return; // 跳过插入数据
      }

      // 更新lastEid
      lastEid = data.eid;

      // 生成动态列
      generateDynamicColumns(data);

      // 插入新数据到表格顶部
      tableData.value.unshift(data);

      // 更新总条数
      totalItems.value = tableData.value.length;

      // 过滤数据
      filterData();
    }
  }
};

// 过滤数据
const filterData = () => {
  filteredData.value = tableData.value.filter(row => {
    const rowTime = new Date(row.etime);
    return (!startTime.value || rowTime >= startTime.value) &&
           (!endTime.value || rowTime <= endTime.value);
  });
  totalItems.value = filteredData.value.length;
};

// 自动刷新数据
const startDataUpdate = () => {
  intervalId = setInterval(() => {
    if (store.getters.selectedDeviceIp !== 'Select An IP') {
      fetchData();
    }
  }, store.getters.intervalTime || 3000);
};

onMounted(() => {
  startDataUpdate();
});

onUnmounted(() => {
  clearInterval(intervalId);
});

// 监听 IP 地址的变化
watch(() => store.getters.selectedDeviceIp, (newIp) => {
  if (newIp !== 'Select An IP') {
    // 当 IP 切换时，立即更新表格
    tableData.value = []; // 清空表格数据
    fetchData(); // 重新加载数据
  } else {
    // IP 为初始值时，清空表格数据
    tableData.value = [];
    filteredData.value = [];
  }
});
</script>


<style scoped>
/* 样式代码保持不变 */
.container {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 20px;
}

.filters {
  display: flex;
  width: 100%;
  max-width: 600px;
  margin-bottom: 20px;
  justify-content: space-between;
}

.filter-item {
  display: flex;
  align-items: center;
  width: 48%;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  padding: 8px 12px;
  box-sizing: border-box;
}

.filter-label {
  margin-right: 8px;
  white-space: nowrap;
}

.inner-picker {
  width: 100%;
}

.table {
  width: 100%;
  max-width: 800px;
  max-height: 700px; /* 设置最大高度 */
  margin-bottom: 20px;
  overflow-y: auto; /* 使表格具有垂直滚动条 */
}

.pagination {
  display: flex;
  justify-content: center;
  width: 100%;
  max-width: 800px;
}
</style>
