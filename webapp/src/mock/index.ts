import Mock from 'mockjs';

// 模拟数据
const data = Mock.mock({
  'threshold': {
    'temperature': '@float(20, 30, 1, 2)', // 生成浮点数
    'smoke': '@float(0, 10, 1, 2)', // 生成浮点数
  },
  'interval': '@integer(5000, 30000)', // 生成 5000 到 30000 毫秒之间的随机整数
  'clientip': [
    '127.0.0.1',
    '192.168.0.2',
    '192.168.156.77'
  ],
  'alarm': {
    '127.0.0.1':"[alarmModule]: No collected data exceeds the threshold, but the alarm lock needs to ensure at least 60 seconds of alarm time.",
    '192.168.0.2':"[alarmModule]: smokeVal is at 2000, which is should be under 2000.\n[alarmModule]: smokeVal is at 2000, which is should be under 2000."
    }
  });

const API_BASE_URL = `${window.location.origin}/api`;

// Mock 接口
Mock.setup({
  timeout: '200-600' // 设置响应时间范围
});

// 模拟 GET 请求
Mock.mock(new RegExp(`${API_BASE_URL}/record`), 'get', (options: any) => {
  // 打印请求的 URL
  console.log('请求 URL:', options.url);

  // 解析查询参数
  const urlParams = new URLSearchParams(options.url.split('?')[1]);
  const ip = urlParams.get('ip');

  console.log('请求参数 ip:', ip);

  // 生成模拟数据
  const datas = Mock.mock({
    'record': { 
      'eid|+1': 1, // 自增的 id，每次请求自增
      'temperatureVal|20-40': 30,  // 随机生成 20 到 40 之间的整数
      'SmokeVal|1000-2000': 1500,  // 随机生成 1000 到 2000 之间的整数
      'HumidityVal|30-50': 40,
      'etime': '@datetime("yyyy-MM-dd HH:mm:ss")',  // 生成随机日期时间
      'note': null,  // 生成 null 值
      'ip': ip || '@ip',  // 如果传入了 ip，则使用传入的 ip，否则随机生成
    }
  });

  return {
    code: 200, 
    data: datas.record
  }
});

Mock.mock(`${API_BASE_URL}/alarm`, 'get', {
  code: 200,
  data: data.alarm
});

Mock.mock(`${API_BASE_URL}/threshold`, 'get', {
  code: 200,
  data: data.threshold
});

Mock.mock(`${API_BASE_URL}/interval`, 'get', {
  code: 200,
  data: data.interval
});

Mock.mock(`${API_BASE_URL}/clientip`, 'get', {
  code: 200,
  data: data.clientip
});
