<template>
  <div class="chart-container">
    <div class="bar-chart">
      <div ref="scatterChartRef" class="echart"></div>
    </div>
    <div class="line-charts">
      <div ref="chartRef" class="echart"></div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, watch } from 'vue';
import { useStore } from 'vuex';
import * as echarts from 'echarts';
import { fetchRealTimeData } from '@/services/api';

const store = useStore();
const chartRef = ref<HTMLElement | null>(null);
const scatterChartRef = ref<HTMLElement | null>(null);
const myChart = ref<echarts.EChartsType | null>(null);
const myScatterChart = ref<echarts.EChartsType | null>(null);
const interval = store.state.intervalTime || 3000; // 获取更新间隔，默认 5000 毫秒

const lineChartData: { [key: string]: { xData: string[]; yData: number[] } } = {}; // 折线图数据缓存
const maxDataPoints = 10; // 最大数据点数量
const initialIp = "Select An IP"; // 初始 IP 值

let currentIp = initialIp; // 当前 IP 值
let lastEid: string | null = null; // 存储上一次获取的数据的eid

async function initChart() {
  if (!chartRef.value) return;
  myChart.value = echarts.init(chartRef.value);
  if (currentIp !== initialIp) {
    updateChart();
  }
}

async function initScatterChart() {
  if (!scatterChartRef.value) return;
  myScatterChart.value = echarts.init(scatterChartRef.value);
  if (currentIp !== initialIp) {
    updateScatterChart();
  }
}

async function updateChart() {
  if (currentIp === initialIp) return; // 如果 IP 是初始值，则不更新图表

  const data = await fetchRealTimeData(currentIp);
  
  // 检查新获取的数据的eid是否与上一次相同
  if (data.eid === lastEid) {
    console.log('数据重复，跳过更新');
    return; // 跳过更新图表
  }

  // 更新 lastEid
  lastEid = data.eid;

  const time = new Date(data.etime).toLocaleTimeString(); // 格式化时间

  // 查找以 'Val' 结尾的字段
  const valTypes = Object.keys(data).filter(key => key.endsWith('Val'));

  valTypes.forEach(valType => {
    if (!lineChartData[valType]) {
      lineChartData[valType] = { xData: [], yData: [] };
    }
    if (lineChartData[valType].xData.length >= maxDataPoints) {
      lineChartData[valType].xData.shift();
      lineChartData[valType].yData.shift();
    }
    lineChartData[valType].xData.push(time);
    lineChartData[valType].yData.push(data[valType]);
  });

  const option = {
    title: { text: 'Real-Time Line Chart' },
    tooltip: { trigger: 'axis' },
    legend: { data: valTypes },
    xAxis: { type: 'category', data: lineChartData[valTypes[0]]?.xData || [] },
    yAxis: { type: 'value' },
    series: valTypes.map(name => ({
      name,
      type: 'line',
      data: lineChartData[name].yData,
    }))
  };

  myChart.value?.setOption(option);
}

async function updateScatterChart() {
  if (currentIp === initialIp) return; // 如果 IP 是初始值，则不更新图表

  const data = await fetchRealTimeData(currentIp);

  // 查找以 'Val' 结尾的字段
  const barData = Object.keys(data)
    .filter(key => key.endsWith('Val'))
    .map(key => ({ value: data[key], name: key }));

  const scatterOption = {
    title: { text: 'Real-Time Bar Chart' },
    tooltip: { trigger: 'item' },
    xAxis: { type: 'value' },
    yAxis: { type: 'category', data: barData.map(d => d.name) },
    series: [{
      type: 'bar',
      data: barData.map(d => d.value),
    }]
  };

  myScatterChart.value?.setOption(scatterOption);
}

function startDataUpdate() {
  setInterval(() => {
    if (currentIp !== initialIp) {
      updateChart();
      updateScatterChart();
    }
  }, interval);
}

onMounted(() => {
  initChart();
  initScatterChart();
  startDataUpdate();
});

watch(() => store.getters.selectedDeviceIp, (newIp) => {
  currentIp = newIp;
  if (currentIp !== initialIp) {
    // 当 IP 切换时，立即更新图表
    Object.keys(lineChartData).forEach(key => {
      lineChartData[key] = { xData: [], yData: [] };
    });
    myChart.value?.clear();
    myScatterChart.value?.clear();
    updateChart();
    updateScatterChart();
  } else {
    // IP 为初始值时，清空图表数据
    if (myChart.value) myChart.value.clear();
    if (myScatterChart.value) myScatterChart.value.clear();
  }
});
</script>


<style scoped>
.chart-container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  padding: 0;
  margin: 0;
}

.bar-chart {
  width: 100%;
  height: 40%;
  padding: 24px 36px;
  box-sizing: border-box;
  position: relative;
}

.echart {
  width: 100%;
  height: 100%;
}

.line-charts {
  display: flex;
  height: 50%;
  width: 100%;
  padding: 24px 36px;
  box-sizing: border-box;
}
</style>
