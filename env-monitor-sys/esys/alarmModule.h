/**
 * @file alarmModule.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief Alert module, checks if the data exceeds the threshold, otherwise starts the alert program
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */


#pragma once
#include <unordered_map>
#include <set>
#include "esysControl.h"  // 包含自定义依赖项

namespace ems {  // 命名空间 ems 开始

	/**
	 * @class alarmModule
	 * @brief 用于管理和监控报警的模块。
	 *
	 * 该类提供了报警监控、阈值管理和报警激活功能，支持多线程环境下的报警状态管理。
	 */
	class alarmModule
	{
	private:
		/**
		 * @brief 报警锁定的持续时间（秒）。
		 */
		static int alarm_lock_duration_seconds;

		/**
		 * @brief 互斥量，用于保护共享数据的访问。
		 */
		static std::mutex mtx;

		/**
		 * @brief 条件变量，用于多线程同步。
		 */
		static std::condition_variable cv;

		/**
		 * @brief 用于重置计时器的标志，key 为 IP 地址，value 为是否重置计时器的原子布尔值。
		 */
		static std::unordered_map<std::string, std::atomic<bool>> reset_timer_active_ip;

		/**
		 * @brief 报警阈值的映射，key 为报警类型，value 为对应的阈值。
		 */
		static std::unordered_map<std::string, double> threshold;

		/**
		 * @brief 客户端报警状态的映射，key 为 IP 地址，value 为是否处于报警状态的原子布尔值。
		 */
		static std::unordered_map<std::string, std::atomic<bool>> alarm_active_ip;

		/**
		 * @brief 客户端报警信息的映射，key 为 IP 地址，value 为一个内部映射，该映射的 key 为报警类型，value 为报警的具体值。
		 */
		static std::unordered_map<std::string, std::unordered_map<std::string, double>> alarm_active_message;

		/**
		 * @brief 报警信息的字符串描述。
		 */
		std::string alarmMessage;

		/**
		 * @brief 私有构造函数，防止类的实例化。
		 */
		alarmModule();

		/**
		 * @brief 私有析构函数。
		 */
		~alarmModule() = default;

		/**
		 * @brief 删除拷贝构造函数，防止拷贝。
		 */
		alarmModule(const alarmModule&) = delete;

		/**
		 * @brief 删除赋值操作符，防止赋值。
		 */
		alarmModule& operator=(const alarmModule&) = delete;

		/**
		 * @brief 用于与 esysControl 类的友元声明。
		 */
		friend class esysControl;

		/**
		 * @brief 监控报警状态。
		 *
		 * @param data 包含监控数据的映射。
		 * @return std::string 报警信息的字符串描述。
		 */
		std::string alarmMonitor(const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief 激活指定客户端 IP 的报警状态。
		 *
		 * @param clientIP 客户端的 IP 地址。
		 */
		static void alarmActive(const std::string& clientIP);

		/**
		 * @brief 存储报警信息的映射，key 为报警类型，value 为报警的具体值。
		 */
		std::map<std::string, std::string> message;

	public:
		/**
		 * @brief 获取 alarmModule 类的单例实例。
		 *
		 * @return alarmModule& 单例实例的引用。
		 */
		static alarmModule& getInstance() {
			static alarmModule instance;
			return instance;
		}

		/**
		 * @brief 获取当前的报警信息。
		 *
		 * @return std::map<std::string, std::string> 报警信息的映射，key 为报警类型，value 为报警的具体值。
		 */
		std::map<std::string, std::string> getAlarmMessage();

		/**
		 * @brief 获取当前设置的报警阈值。
		 *
		 * @return std::unordered_map<std::string, double> 报警阈值的映射，key 为报警类型，value 为阈值。
		 */
		std::unordered_map<std::string, double> getThreshold();
	};
} // namespace ems 结束
