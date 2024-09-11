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
        // ��������
        std::unordered_map<std::string, std::string> config;
        const std::string& configFilePath;
        std::string logFilePath;

        // ˽�й��캯��
        esysControl();

        // ˽�е���������
        ~esysControl() = default;

        // ɾ���������캯���͸�ֵ������
        esysControl(const esysControl&) = delete;
        esysControl& operator=(const esysControl&) = delete;

        // ��ȡ�����ļ�������
        void loadConfig(const std::string& filePath);

        void defaultConfig(const std::string& filePath);

        // ���������ڵ��ļ���
        void create_directory_if_not_exists(const std::filesystem::path& dir);

        // ͨ���̳�streambuf����дϵͳ��overflow��xsputn����
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

        // ��־����
        std::ofstream logFile;
        LogStreamBuf* logStreamBuf;

        // ��ȡ��ǰ����
        std::string getCurrentDateAsYYmmdd();

        // ����log�ļ���
        std::string findNextLogFileName(const std::string& logsDirectory);

        // ������־�ļ����ض������
        void setupLogging();

        static void runTcpServer(std::shared_mutex& mtx);

        static void runHttpServer(std::shared_mutex& mtx);


    public:
        // ��ȡ����ʵ��
        static esysControl& getInstance() {
            static esysControl instance;
            return instance;
        }

        int sysRun();

        // ��ȡ������
        std::string getConfig(const std::string& key) const;

        // ��ȡ����������м���Ϣ
        std::vector<std::string> getAllConfigKeys() const;

        // ������Ϣ
        std::string messageHandle(const std::string& clientIP, const std::string& request);
    };

}  // namespace ems