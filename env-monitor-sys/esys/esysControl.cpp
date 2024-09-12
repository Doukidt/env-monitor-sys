#include "esysControl.h"

namespace ems {

	// 私有构造函数
	esysControl::esysControl() : configFilePath("configs/esys.conf"), logStreamBuf(nullptr), logFilePath("logs/") {
		// 加载日志文件
		setupLogging();
		// 加载配置文件
		loadConfig(configFilePath);
	}

	void esysControl::create_directory_if_not_exists(const std::filesystem::path& dir) {
		std::filesystem::path dirPath = dir.parent_path();
		try {
			if (!std::filesystem::exists(dirPath)) {
				std::filesystem::create_directories(dirPath);
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "[esysControl]: Filesystem error: " << e.what() << std::endl;
		}
	}

	// 读取配置文件并解析
	void esysControl::loadConfig(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			std::cerr << "[esysControl]: Failed to find config file at " << filePath << "." << std::endl;
			defaultConfig(filePath);
			return;
		}

		std::string line;
		while (std::getline(file, line)) {
			// 查找并去除注释
			size_t commentPos = line.find('#');
			if (commentPos != std::string::npos) {
				line = line.substr(0, commentPos);  // 只保留注释符号前的内容
			}

			// 去除行首尾的空白字符
			std::istringstream iss(line);
			std::string key, value;

			// 读取键值对
			if (std::getline(std::getline(iss, key, '=') >> std::ws, value)) {
				// 去除键与值中的前后空白字符
				key.erase(key.find_last_not_of(" \t") + 1);
				value.erase(0, value.find_first_not_of(" \t"));
				value.erase(value.find_last_not_of(" \t") + 1);
				config[key] = value;
			}
		}
		std::cout << "[esysControl]: Read config file successfully." << std::endl;
		file.close();
	}

	void esysControl::defaultConfig(const std::string& filePath) {
		create_directory_if_not_exists(filePath);
		std::ofstream file(filePath, std::ios::binary);
		if (!file) {
			std::cerr << "[esysControl]: Error: Failed to create config file." << std::endl;
			return;
		}
		std::vector<std::string> default_config = {
			"# tcp server's ip address and port",
			"tcp_server_ip = 127.0.0.1",	
			"tcp_server_port = 8080",
			"# the database settings",
			"db_url = tcp://127.0.0.1:3306",
			"db_user = root",
			"db_password = 1234",
			"db_schema = envdb",
			"db_build_file_location = ./envdb.sql",
			"suffix_of_collected_values = Val",
			"# the http server settings",
			"hs_host = 127.0.0.1",
			"hs_port = 5050",
			"hs_mount_dir = ./dist"
			"# alarm program settings",
			"prefix_of_threshold_value = threshold_",
			"threshold_temperature = ",
			"threshold_humidity = ",
			"threshold_smoke = ",
			"alarm_lock_duration_seconds = 60",
			"# log settings",
			"log_operations = false"
		};
		for (int index = 0; index < default_config.size(); index++) {
			file << default_config[index] << std::endl;
		}

		file.close();
		std::cout << "[esysControl]: Create config file using system default initialization at \"configs/esys.conf\"." << std::endl;
		loadConfig(filePath);
	}

	esysControl::LogStreamBuf::LogStreamBuf(std::streambuf* consoleBuf, std::ofstream& logFile)
		: oldConsoleBuf(consoleBuf), logStream(logFile) {}

	// LogStreamBuf 的 overflow 函数
	int esysControl::LogStreamBuf::overflow(int ch) {
		if (ch != EOF) {
			std::lock_guard<std::mutex> lock(mutex_);  // 保护对 shared 资源的访问 
			if (!toggleLineBreakFlag(false)) {
				if (ch == '\n') toggleLineBreakFlag(true);
			}
			if (logStream.is_open()) {
				logStream.put(static_cast<char>(ch));  // 写入日志文件
				logStream.flush();  // 确保数据立即刷新到文件
			}
			if (oldConsoleBuf) {
				oldConsoleBuf->sputc(static_cast<char>(ch));  // 写入控制台
			}
		}
		return ch;
	}

	// LogStreamBuf 的 xsputn 函数
	std::streamsize esysControl::LogStreamBuf::xsputn(const char* s, std::streamsize n) {
		std::lock_guard<std::mutex> lock(mutex_);  // 使用互斥锁保护
		std::string sow;
		if (toggleLineBreakFlag(false)) {
			sow = "[" + getCurrentTimeAsHHMMSS() + "]" + s;
			if (s[n - 1] != '\n')	toggleLineBreakFlag(true);
		}
		else {
			sow = s;
			if (s[n - 1] == '\n')	toggleLineBreakFlag(true);
		}
		if (logStream.is_open()) {
			logStream.write(sow.c_str(), sow.size());  // 写入日志文件
			logStream.flush();  // 确保数据立即刷新到文件
		}
		if (oldConsoleBuf) {
			oldConsoleBuf->sputn(sow.c_str(), sow.size());  // 写入控制台
		}
		return n;
	}

	std::string esysControl::LogStreamBuf::getCurrentTimeAsHHMMSS() {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm local_time;
		if (localtime_s(&local_time, &now_time_t) != 0) {
			std::cerr << "[esysControl]: Error converting time to local time." << std::endl;
			return "";
		}
		std::ostringstream oss;
		oss << std::put_time(&local_time, "%H:%M:%S");
		return oss.str();
	}

	bool esysControl::LogStreamBuf::toggleLineBreakFlag(bool is_toggle) {
		static bool line_break_flag = true;
		line_break_flag = is_toggle ? !line_break_flag : line_break_flag;
		return line_break_flag;
	}

	std::string esysControl::getCurrentDateAsYYmmdd() {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm local_time;
		// 使用 localtime_s 进行线程安全的本地时间转换
		if (localtime_s(&local_time, &now_time_t) != 0) {
			std::cerr << "[esysControl]: Error converting time to local time." << std::endl;
			return "";
		}
		std::ostringstream oss;
		oss << std::put_time(&local_time, "%Y-%m-%d");  // 格式化为 YYYY-MM-DD
		return oss.str();
	}


	std::string esysControl::findNextLogFileName(const std::string& logFilePath) {
		create_directory_if_not_exists(logFilePath);
		std::string currentDate = getCurrentDateAsYYmmdd(); // 获取当前日期
		int maxIndex = 0;  // 存储当前日期的最大索引

		// 使用正则表达式匹配 YYYY-MM-DD-index.log 格式的文件
		std::regex logFilePattern(currentDate + R"(-(\d+)\.log)");

		// 遍历 logs 文件夹中的所有文件
		for (const auto& entry : std::filesystem::directory_iterator(logFilePath)) {
			if (entry.is_regular_file()) {
				std::string fileName = entry.path().filename().string();
				std::smatch match;
				if (std::regex_match(fileName, match, logFilePattern)) {
					int index = std::stoi(match[1].str());
					maxIndex = max(maxIndex, index); // 找到最大的索引
				}
			}
		}
		// 生成下一个日志文件名
		int nextIndex = maxIndex + 1;
		std::ostringstream newLogFileName;
		newLogFileName << logFilePath << currentDate << "-" << nextIndex << ".log";
		return newLogFileName.str();
	}

	void esysControl::setupLogging() {
		if (logFilePath == "logs/") {
			logFilePath = findNextLogFileName(logFilePath);
		}

		// 关闭之前的日志文件（如果有的话）
		if (logFile.is_open()) {
			logFile.close();
		}

		// 打开新的日志文件
		logFile.open(logFilePath, std::ios::out | std::ios::binary);
		if (!logFile) {
			std::cerr << "[esysControl]: Error: Failed to open log file." << std::endl;
			return;
		}

		// 创建 LogStreamBuf 对象并设置流缓冲区
		if (logStreamBuf) {
			delete logStreamBuf;
		}
		logStreamBuf = new LogStreamBuf(std::cout.rdbuf(), logFile);
		std::cout.rdbuf(logStreamBuf);
		std::cerr.rdbuf(logStreamBuf);
	}

	void esysControl::runTcpServer(std::shared_mutex& mtx)
	{
		tcpConnector conn(mtx);
		conn.startServer([](const std::string& clientIP, const std::string& request) -> std::string {
			esysControl& esys = esysControl::getInstance();
			return esys.messageHandle(clientIP, request);
			});
	}

	void esysControl::runHttpServer(std::shared_mutex& mtx)
	{
		httpServer hvr(mtx);
		hvr.run();
	}


	int esysControl::sysRun()
	{
		// 初始化数据库连接
		dbTools::getInstance();
		// 初始化报警模块
		alarmModule::getInstance();

		std::shared_mutex mtx;
		
		std::thread httpServer(runHttpServer, std::ref(mtx));

		std::thread tcpServer(runTcpServer, std::ref(mtx));
		if(httpServer.joinable())	httpServer.join();
		if(tcpServer.joinable())	tcpServer.join();
		return 0;
	}

	// 获取配置项
	std::string esysControl::getConfig(const std::string& key) const {
		auto it = config.find(key);
		if (it != config.end()) {
			return it->second;
		}
		return "";  // 或者抛出异常，根据需求决定
	}

	std::vector<std::string> esysControl::getAllConfigKeys() const
	{
		std::vector<std::string> keys;
		keys.reserve(config.size());

		// 遍历config中的所有键，并将它们添加到keys向量中
		for (const auto& pair : config) {
			keys.push_back(pair.first);
		}
		return keys;
	}

	// 处理消息
	std::string esysControl::messageHandle(const std::string& clientIP, const std::string& request) {
		dbTools& db = dbTools::getInstance();
		alarmModule& am = alarmModule::getInstance();

		std::unordered_map<std::string, std::string> dataMap;
		dataMap["clientIP"] = clientIP;

		// 正则表达式匹配键值对
		std::regex keyValuePattern(R"+((\\"|\\')(\w+)(\\"|\\')\s*:\s*(\d+\.?\d*))+");
		auto wordsBegin = std::sregex_iterator(request.begin(), request.end(), keyValuePattern);
		auto wordsEnd = std::sregex_iterator();

		for (std::sregex_iterator i = wordsBegin; i != wordsEnd; ++i) {
			std::smatch match = *i;
			std::string key = match[2].str();
			std::string value = match[4].str();
			dataMap[key] = value;
		}

		// 插入数据到数据库
		dataMap["etime"] = "NOW()";
		std::string table_name = "envtable";
		int result = db.dbInsert(table_name, dataMap);

		return am.alarmMonitor(dataMap);
	}

}  // namespace ems
