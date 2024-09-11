// src/services/api.ts
import axios from 'axios';

// API 基础 URL（根据实际情况调整）
const API_BASE_URL = `${window.location.origin}/api`;

// 获取采集设备ip
export const fetchExistIP = async () => {
  try {
    const response = await axios.get(`${API_BASE_URL}/clientip`);
    return response.data;
  } catch (error) {
    console.error('Error fetching real-time data:', error);
    throw error;
  }
};

// 获取实时数据
export const fetchRealTimeData = async (ip: string) => {
  try {
    const response = await axios.get(`${API_BASE_URL}/record`, {
      params: { ip },
    });
    if (response.data && response.data.code === 200) {  // 确保响应数据结构正确
      let data = response.data.data;
    
      data = Object.fromEntries(
        Object.entries(data).filter(
          ([key, value]) => !(key.endsWith('Val') && (value === null || value === undefined || value === ''))
        )
      );

      return data; // 返回处理后的数据
    } else {
      console.error('Unexpected response format:', response.data);
      return null;
    }
  } catch (error) {
    console.error('Error fetching real-time data:', error);
    throw error;
  }
};

// 获取阈值数据
export const fetchThresholds = async () => {
  try {
    const response = await axios.get(`${API_BASE_URL}/thresholds`);
    return response.data;
  } catch (error) {
    console.error('Error fetching thresholds:', error);
    throw error;
  }
};

// 获取数据采集时间间隔
export const fetchInterval = async () => {
  try {
    const response = await axios.get(`${API_BASE_URL}/interval`);
    return response.data.interval;
  } catch (error) {
    console.error('Error fetching interval:', error);
    throw error;
  }
};

// 获取警告信息
export const fetchAlarms = async () => {
  try {
    const response = await axios.get(`${API_BASE_URL}/alarm`);
    return response.data;
  } catch (error) {
    console.error('Error fetching alarms:', error);
    throw error;
  }
};
