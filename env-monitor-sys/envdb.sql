CREATE DATABASE IF NOT EXISTS envdb;
USE envdb;

CREATE TABLE envtable(
  eid INT PRIMARY KEY AUTO_INCREMENT COMMENT '主键自增',
  temperatureVal FLOAT COMMENT '温度值',
  humidityVal FLOAT COMMENT '湿度值',
  smokeVal INT COMMENT '气味值',
  clientIP CHAR(16) COMMENT '采集设备ip地址',
  etime DATETIME COMMENT '录入信息时间',
  note VARCHAR(150) COMMENT '备注'
);