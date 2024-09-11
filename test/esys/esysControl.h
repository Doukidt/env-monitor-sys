#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <regex>
#include <mutex>  
#include <shared_mutex>
#include "../db/dbTools.h"
#include "../network/tcpConnector.h"
#include "../network/httpServer.h"
#include "alarmModule.h"

namespace ems {

    class esysControl {
    private:
        // 配置数据
        std::unordered_map<std::string, std::string> config;
        const std::string& configFilePath;
        std::string logFilePath;

        // 私有构造函数
        esysControl();

        // 私有的析构函数
        ~esysControl() = default;

        // 删除拷贝构造函数和赋值操作符
        esysControl(const esysControl&) = delete;
        esysControl& operator=(const esysControl&) = delete;

        // 读取配置文件并解析
        void loadConfig(const std::string& filePath);

        void defaultConfig(const std::string& filePath);

        // 创建不存在的文件夹
        void create_directory_if_not_exists(const std::filesystem::path& dir);

        // 通过继承streambuf类重写系统的overflow和xsputn方法
        class LogStreamBuf : public std::streambuf {
        public:
            LogStreamBuf(std::streambuf* consoleBuf, std::ofstream& logFile);

        protected:
            int overflow(int ch) override;
            std::streamsize xsputn(const char* s, std::streamsize n) override;

        private:
            std::string getCurrentTimeAsHHMMSS();
            static bool toggleLineBreakFlag(bool is_toggle);

            std::streambuf* oldConsoleBuf;
            std::ofstream& logStream;
            std::mutex mutex_;
        };

        // 日志数据
        std::ofstream logFile;
        LogStreamBuf* logStreamBuf;

        // 获取当前日期
        std::string getCurrentDateAsYYmmdd();

        // 查找log文件名
        std::string findNextLogFileName(const std::string& logsDirectory);

        // 设置日志文件和重定向输出
        void setupLogging();

        static void runTcpServer(std::shared_mutex& mtx);

        static void runHttpServer(std::shared_mutex& mtx);


    public:
        // 获取单例实例
        static esysControl& getInstance() {
            static esysControl instance;
            return instance;
        }

        int sysRun();

        // 获取配置项
        std::string getConfig(const std::string& key) const;

        // 获取配置项的所有键信息
        std::vector<std::string> getAllConfigKeys() const;

        // 处理消息
        std::string messageHandle(const std::string& clientIP, const std::string& request);
    };

}  // namespace ems