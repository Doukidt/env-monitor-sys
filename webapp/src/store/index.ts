// src/store/index.ts
import { createStore } from 'vuex';
import { Document, Menu as IconMenu } from '@element-plus/icons-vue';

export default createStore({
  state() {
    return {
      alertMessage: '',  // 提示消息
      menu: [  // 菜单
        {
          name: 'Charts',
          path: '/chart',
          icon: IconMenu,
        },
        {
          name: 'Table',
          path: '/table',
          icon: Document
        }
      ],
      selectedDeviceIp: 'Select An IP', // 选中的设备 IP 默认值
      intervalTime: 3000, // 数据刷新时间间隔（以毫秒为单位）
    };
  },

  mutations: {
    setAlertMessage(state, message: string) { // 更新提示消息
      state.alertMessage = message;
    },
    setSelectedDeviceIp(state, ip: string) { // 更新选中的设备 IP
      state.selectedDeviceIp = ip;
    },
    setIntervalTime(state, time: number) { // 更新时间间隔
      state.intervalTime = time;
    },
  },

  actions: {
    showAlert({ commit }, message: string) { // 显示提示信息
      commit('setAlertMessage', message);
    },
    updateSelectedDeviceIp({ commit }, ip: string) { // 更新选中的设备 IP
      commit('setSelectedDeviceIp', ip);
    },
    updateIntervalTime({ commit }, time: number) { // 更新时间间隔
      commit('setIntervalTime', time);
    },
  },

  getters: {
    alertMessage(state) { // 获取当前提示消息
      return state.alertMessage;
    },
    selectedDeviceIp(state) { // 获取当前选中的设备 IP
      return state.selectedDeviceIp;
    },
    intervalTime(state) { // 获取当前时间间隔
      return state.intervalTime;
    },
  },
});
