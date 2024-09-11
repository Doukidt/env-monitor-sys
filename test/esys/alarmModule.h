#pragma once
#include <unordered_map>
#include <set>
#include "esysControl.h"

namespace ems {
	class alarmModule
	{
	private:
		static int alarm_lock_duration_seconds;
		static std::mutex mtx;                   // 互斥量用于保护对shared data的访问
		static std::condition_variable cv;      // 条件变量用于同步

		static std::unordered_map<std::string, std::atomic<bool>> reset_timer_active_ip;	// 用于重置计时器的标志
		static std::unordered_map<std::string, double> threshold;
		static std::unordered_map<std::string, std::atomic<bool>> alarm_active_ip;	// 客户端报警位
		static std::unordered_map<std::string, std::unordered_map<std::string, double>> alarm_active_message; // 客户端报警信息
		std::string alarmMessage;

		alarmModule();

		~alarmModule() = default;

		alarmModule(const alarmModule&) = delete;
		alarmModule& operator=(const alarmModule&) = delete;
		
		friend class esysControl;

		std::string alarmMonitor(const std::unordered_map<std::string, std::string>& data);

		static void alarmActive(const std::string& clientIP);

		std::map<std::string, std::string> message;

	public:
		static alarmModule& getInstance() {
			static alarmModule instance;
			return instance;
		}

		std::map<std::string, std::string> getAlarmMessage();
	};
} // namespace ems

