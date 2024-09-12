/**
 * @file esysControl.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief System control module for managing configuration, logging, and server operations.
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */
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

    /**
     * @class esysControl
     * @brief 用于管理配置、日志记录和服务器操作的主要控制模块。
     */
    class esysControl {
    private:
        std::unordered_map<std::string, std::string> config;       ///< 配置信息数据。
        const std::string& configFilePath;                         ///< 配置文件的路径。
        std::string logFilePath;                                   ///< 日志文件的路径。

        /**
         * @brief 私有构造函数，防止实例化。
         */
        esysControl();

        /**
         * @brief 私有析构函数。
         */
        ~esysControl() = default;

        /**
         * @brief 删除的拷贝构造函数，防止复制。
         */
        esysControl(const esysControl&) = delete;

        /**
         * @brief 删除的赋值操作符，防止赋值。
         */
        esysControl& operator=(const esysControl&) = delete;

        /**
         * @brief 读取并解析配置文件。
         *
         * @param filePath 配置文件的路径。
         */
        void loadConfig(const std::string& filePath);

        /**
         * @brief 如果配置文件不存在，则设置默认配置。
         *
         * @param filePath 配置文件的路径。
         */
        void defaultConfig(const std::string& filePath);

        /**
         * @brief 如果目录不存在则创建该目录。
         *
         * @param dir 要创建的目录路径。
         */
        void create_directory_if_not_exists(const std::filesystem::path& dir);

        /**
         * @class LogStreamBuf
         * @brief 自定义流缓冲区，将输出重定向到控制台和日志文件。
         */
        class LogStreamBuf : public std::streambuf {
        public:
            /**
             * @brief LogStreamBuf 的构造函数。
             *
             * @param consoleBuf 原始控制台缓冲区的指针。
             * @param logFile 日志文件流的引用。
             */
            LogStreamBuf(std::streambuf* consoleBuf, std::ofstream& logFile);

        protected:
            /**
             * @brief 重写的 overflow 函数，用于处理单字符输出。
             *
             * @param ch 要输出的字符。
             * @return int 输出字符或错误时返回 EOF。
             */
            int overflow(int ch) override;

            /**
             * @brief 重写的 xsputn 函数，用于处理多字符输出。
             *
             * @param s 字符数组的指针。
             * @param n 要写入的字符数量。
             * @return std::streamsize 写入的字符数量。
             */
            std::streamsize xsputn(const char* s, std::streamsize n) override;

        private:
            /**
             * @brief 获取当前时间作为 HH:MM:SS 格式的字符串。
             *
             * @return std::string 当前时间，格式为 HH:MM:SS。
             */
            std::string getCurrentTimeAsHHMMSS();

            /**
             * @brief 切换换行标志。
             *
             * @param is_toggle 表示是否切换的布尔值。
             * @return bool 新的换行标志状态。
             */
            static bool toggleLineBreakFlag(bool is_toggle);

            std::streambuf* oldConsoleBuf;      ///< 原始控制台缓冲区的指针。
            std::ofstream& logStream;           ///< 日志文件流的引用。
            std::mutex mutex_;                  ///< 用于同步访问流缓冲区的互斥锁。
        };

        std::ofstream logFile;                  ///< 用于记录数据的流对象。
        LogStreamBuf* logStreamBuf;             ///< 自定义日志流缓冲区的指针。

        /**
         * @brief 获取当前日期，格式为 YYMMDD 的字符串。
         *
         * @return std::string 当前日期，格式为 YYMMDD。
         */
        std::string getCurrentDateAsYYmmdd();

        /**
         * @brief 在指定目录中找到下一个可用的日志文件名。
         *
         * @param logsDirectory 日志文件存储的目录。
         * @return std::string 下一个可用的日志文件名。
         */
        std::string findNextLogFileName(const std::string& logsDirectory);

        /**
         * @brief 通过配置日志文件和重定向输出来设置日志记录。
         */
        void setupLogging();

        /**
         * @brief 在单独的线程中运行 TCP 服务器。
         *
         * @param mtx 用于同步服务器操作的共享互斥锁。
         */
        static void runTcpServer(std::shared_mutex& mtx);

        /**
         * @brief 在单独的线程中运行 HTTP 服务器。
         *
         * @param mtx 用于同步服务器操作的共享互斥锁。
         */
        static void runHttpServer(std::shared_mutex& mtx);

    public:
        /**
         * @brief 获取 esysControl 的单例实例。
         *
         * @return esysControl& 单例实例的引用。
         */
        static esysControl& getInstance() {
            static esysControl instance;
            return instance;
        }

        /**
         * @brief 启动系统运行循环。
         *
         * @return int 操作的状态码。
         */
        int sysRun();

        /**
         * @brief 通过键获取配置信息的值。
         *
         * @param key 配置的键。
         * @return std::string 配置的值。
         */
        std::string getConfig(const std::string& key) const;

        /**
         * @brief 获取所有配置键。
         *
         * @return std::vector<std::string> 所有配置键的向量。
         */
        std::vector<std::string> getAllConfigKeys() const;

        /**
         * @brief 处理来自客户端的消息。
         *
         * @param clientIP 客户端的 IP 地址。
         * @param request 请求消息。
         * @return std::string 对客户端请求的响应。
         */
        std::string messageHandle(const std::string& clientIP, const std::string& request);
    };

}  // namespace ems
