#include "alarmModule.h"

namespace ems {

	std::mutex alarmModule::mtx;
	std::condition_variable alarmModule::cv;
	int alarmModule::alarm_lock_duration_seconds = 10;
	std::unordered_map<std::string, double> alarmModule::threshold;
	std::unordered_map<std::string, std::atomic<bool>> alarmModule::reset_timer_active_ip;
	std::unordered_map<std::string, std::atomic<bool>> alarmModule::alarm_active_ip;
	std::unordered_map<std::string, std::unordered_map<std::string, double>> alarmModule::alarm_active_message;

	alarmModule::alarmModule(): alarmMessage("") {
		esysControl& esys = esysControl::getInstance();
		alarm_lock_duration_seconds = std::stoi(esys.getConfig("alarm_lock_duration_seconds"));
		std::string prefix_of_threshold_value = esys.getConfig("prefix_of_threshold_value");
		size_t prefix_of_threshold_length = prefix_of_threshold_value.length();
		// 获取所有配置项的键
		std::vector<std::string> configKeys = esys.getAllConfigKeys();

		for (const auto& key : configKeys) {
			if (key.find(prefix_of_threshold_value) == 0) {  // 检查键是否以 "threshold_" 开头
				std::string param = key.substr(prefix_of_threshold_length);  // 获取 "threshold_" 后面的部分，如 "temperature", "humidity"
				std::string value = esys.getConfig(key);  

				if (!value.empty()) {  
					threshold[param] = std::stod(value);  
				}
			}
		}

	}

	std::string alarmModule::alarmMonitor(const std::unordered_map<std::string, std::string>& data)
	{
		bool tmpalarm = false;
		std::string clientIP = data.at("clientIP");
		esysControl& esys = esysControl::getInstance();
		std::string suffix_of_collected_values  = esys.getConfig("suffix_of_collected_values");
		try {
			for (auto& thr : threshold) {
				if (std::stod(data.at(thr.first + suffix_of_collected_values)) >= thr.second) tmpalarm = true;
			}
		}
		catch (const std::out_of_range& e) {
			std::cerr << "[alarmModule]: Key not found in data: " << e.what() << std::endl;
			return "error";
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "[alarmModule]: Invalid argument in data: " << e.what() << std::endl;
			return "error";
		}

		if (tmpalarm || alarm_active_ip[clientIP]) {
			std::unordered_map<std::string, double> alarm_data;
			for (auto& thr : threshold) {
				alarm_data[thr.first] = std::stod(data.at(thr.first + suffix_of_collected_values));
			}
			alarm_active_message[clientIP] = alarm_data;
			if (!alarm_active_ip[clientIP]) {
				std::thread alarmThread(alarmActive, clientIP);
				alarmThread.detach();
				while (!alarm_active_ip[clientIP]);
			}
			else if (tmpalarm && alarm_active_ip[clientIP]) {
				std::lock_guard<std::mutex> lock(mtx);
				reset_timer_active_ip[clientIP] = true;
			}
			std::cout << "[alarmModule]: Alarm is atcive now at [" + clientIP + "]." << std::endl;
			bool val_not_under_threshold_exist = false;
			std::stringstream ss;
			for (auto& it : alarm_active_message[clientIP]) {
				if (it.second >= threshold[it.first]) {
					val_not_under_threshold_exist = true;
					ss << "[alarmModule]: " + it.first + suffix_of_collected_values + " is at " << it.second
						<< ", which is should be under " << threshold[it.first] << "." << std::endl;
				}
			}
			if (!val_not_under_threshold_exist) {
				ss << "[alarmModule]: No collected data exceeds the threshold, " <<
					"but the alarm lock needs to ensure at least "<< alarm_lock_duration_seconds << " seconds of alarm time." << std::endl;
			}
			alarmMessage = ss.str();
			message[clientIP] = alarmMessage;
			return "alarm_active";
		}
		else {
			if (alarm_active_message.count(clientIP) > 0) {
				alarm_active_message.erase(clientIP);
				message.erase(clientIP);
			}
		}

		return "ack";
	}

	void alarmModule::alarmActive(const std::string& clientIP)
	{
		using namespace std::chrono;

		auto start_time = steady_clock::now();
		auto end_time = start_time + seconds(static_cast<int>(alarm_lock_duration_seconds));

		{
			std::lock_guard<std::mutex> lock(mtx);
			alarm_active_ip[clientIP] = true;
		}

		while (steady_clock::now() < end_time) {
			// 检查是否需要重置计时器
			if (reset_timer_active_ip[clientIP]) {
				start_time = steady_clock::now();
				end_time = start_time + seconds(static_cast<int>(alarm_lock_duration_seconds));
				reset_timer_active_ip[clientIP] = false;  // 重置标志
			}

			std::this_thread::sleep_for(milliseconds(100));  // 检查间隔
		}

		{
			std::lock_guard<std::mutex> lock(mtx);
			alarm_active_ip[clientIP] = false;
		}
	}

	std::map<std::string, std::string> alarmModule::getAlarmMessage()
	{
		return message;
	}

	std::unordered_map<std::string, double> alarmModule::getThreshold()
	{
		return threshold;
	}

} // namespace ems