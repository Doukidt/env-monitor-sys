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
     * @brief ���ڹ������á���־��¼�ͷ�������������Ҫ����ģ�顣
     */
    class esysControl {
    private:
        std::unordered_map<std::string, std::string> config;       ///< ������Ϣ���ݡ�
        const std::string& configFilePath;                         ///< �����ļ���·����
        std::string logFilePath;                                   ///< ��־�ļ���·����

        /**
         * @brief ˽�й��캯������ֹʵ������
         */
        esysControl();

        /**
         * @brief ˽������������
         */
        ~esysControl() = default;

        /**
         * @brief ɾ���Ŀ������캯������ֹ���ơ�
         */
        esysControl(const esysControl&) = delete;

        /**
         * @brief ɾ���ĸ�ֵ����������ֹ��ֵ��
         */
        esysControl& operator=(const esysControl&) = delete;

        /**
         * @brief ��ȡ�����������ļ���
         *
         * @param filePath �����ļ���·����
         */
        void loadConfig(const std::string& filePath);

        /**
         * @brief ��������ļ������ڣ�������Ĭ�����á�
         *
         * @param filePath �����ļ���·����
         */
        void defaultConfig(const std::string& filePath);

        /**
         * @brief ���Ŀ¼�������򴴽���Ŀ¼��
         *
         * @param dir Ҫ������Ŀ¼·����
         */
        void create_directory_if_not_exists(const std::filesystem::path& dir);

        /**
         * @class LogStreamBuf
         * @brief �Զ�������������������ض��򵽿���̨����־�ļ���
         */
        class LogStreamBuf : public std::streambuf {
        public:
            /**
             * @brief LogStreamBuf �Ĺ��캯����
             *
             * @param consoleBuf ԭʼ����̨��������ָ�롣
             * @param logFile ��־�ļ��������á�
             */
            LogStreamBuf(std::streambuf* consoleBuf, std::ofstream& logFile);

        protected:
            /**
             * @brief ��д�� overflow ���������ڴ����ַ������
             *
             * @param ch Ҫ������ַ���
             * @return int ����ַ������ʱ���� EOF��
             */
            int overflow(int ch) override;

            /**
             * @brief ��д�� xsputn ���������ڴ�����ַ������
             *
             * @param s �ַ������ָ�롣
             * @param n Ҫд����ַ�������
             * @return std::streamsize д����ַ�������
             */
            std::streamsize xsputn(const char* s, std::streamsize n) override;

        private:
            /**
             * @brief ��ȡ��ǰʱ����Ϊ HH:MM:SS ��ʽ���ַ�����
             *
             * @return std::string ��ǰʱ�䣬��ʽΪ HH:MM:SS��
             */
            std::string getCurrentTimeAsHHMMSS();

            /**
             * @brief �л����б�־��
             *
             * @param is_toggle ��ʾ�Ƿ��л��Ĳ���ֵ��
             * @return bool �µĻ��б�־״̬��
             */
            static bool toggleLineBreakFlag(bool is_toggle);

            std::streambuf* oldConsoleBuf;      ///< ԭʼ����̨��������ָ�롣
            std::ofstream& logStream;           ///< ��־�ļ��������á�
            std::mutex mutex_;                  ///< ����ͬ���������������Ļ�������
        };

        std::ofstream logFile;                  ///< ���ڼ�¼���ݵ�������
        LogStreamBuf* logStreamBuf;             ///< �Զ�����־����������ָ�롣

        /**
         * @brief ��ȡ��ǰ���ڣ���ʽΪ YYMMDD ���ַ�����
         *
         * @return std::string ��ǰ���ڣ���ʽΪ YYMMDD��
         */
        std::string getCurrentDateAsYYmmdd();

        /**
         * @brief ��ָ��Ŀ¼���ҵ���һ�����õ���־�ļ�����
         *
         * @param logsDirectory ��־�ļ��洢��Ŀ¼��
         * @return std::string ��һ�����õ���־�ļ�����
         */
        std::string findNextLogFileName(const std::string& logsDirectory);

        /**
         * @brief ͨ��������־�ļ����ض��������������־��¼��
         */
        void setupLogging();

        /**
         * @brief �ڵ������߳������� TCP ��������
         *
         * @param mtx ����ͬ�������������Ĺ���������
         */
        static void runTcpServer(std::shared_mutex& mtx);

        /**
         * @brief �ڵ������߳������� HTTP ��������
         *
         * @param mtx ����ͬ�������������Ĺ���������
         */
        static void runHttpServer(std::shared_mutex& mtx);

    public:
        /**
         * @brief ��ȡ esysControl �ĵ���ʵ����
         *
         * @return esysControl& ����ʵ�������á�
         */
        static esysControl& getInstance() {
            static esysControl instance;
            return instance;
        }

        /**
         * @brief ����ϵͳ����ѭ����
         *
         * @return int ������״̬�롣
         */
        int sysRun();

        /**
         * @brief ͨ������ȡ������Ϣ��ֵ��
         *
         * @param key ���õļ���
         * @return std::string ���õ�ֵ��
         */
        std::string getConfig(const std::string& key) const;

        /**
         * @brief ��ȡ�������ü���
         *
         * @return std::vector<std::string> �������ü���������
         */
        std::vector<std::string> getAllConfigKeys() const;

        /**
         * @brief �������Կͻ��˵���Ϣ��
         *
         * @param clientIP �ͻ��˵� IP ��ַ��
         * @param request ������Ϣ��
         * @return std::string �Կͻ����������Ӧ��
         */
        std::string messageHandle(const std::string& clientIP, const std::string& request);
    };

}  // namespace ems
