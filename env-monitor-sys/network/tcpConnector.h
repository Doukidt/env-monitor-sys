/**
 * @file tcpConnector.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief The TCP connector module is used to manage TCP server operations, 
 *  including handling client connections and data exchange.
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */

#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>  // For inet_ntop
#include <shared_mutex>
#include "../esys/esysControl.h"
#pragma comment(lib, "ws2_32.lib")

static constexpr int BUFFER_SIZE = 1024;  ///< ��������С�����ڽ������ݡ�

namespace ems {

    /**
     * @class tcpConnector
     * @brief ����TCP�����������Ӻ����ݽ�����
     */
    class tcpConnector {
    public:
        /**
         * @brief ���캯������ʼ��TCP��������
         *
         * @param mtx ������������ͬ��������������
         */
        tcpConnector(std::shared_mutex& mtx);

        /**
         * @brief �����������رշ�������������Դ��
         */
        ~tcpConnector();

        /**
         * @brief ����TCP��������
         *
         * @param handleFunction ����ָ�룬���ڴ�����յ���TCP��Ϣ��
         * @return int ����������ɹ�����0��ʧ�ܷ���1��
         */
        int startServer(std::string(*handleFunction)(const std::string&, const std::string&));

    private:
        unsigned short port;                            ///< �����������˿ڡ�
        SOCKET serverSocket;                            ///< �������׽��֡�
        std::vector<std::thread> threads;               ///< �̳߳أ����ڴ���ͻ������ӡ�
        std::shared_mutex& mtx;                         ///< ������������ͬ��������
        bool log_operations;                            ///< �Ƿ��¼������־�ı�־��
        std::vector<std::string> all_client_ip;         ///< �������ӵĿͻ���IP��ַ�б�

        /**
         * @brief ��ʼ��Winsock�⡣
         *
         * @return bool ����������ɹ�����true��ʧ�ܷ���false��
         */
        bool initializeWinsock();

        /**
         * @brief �����������׽��֡�
         *
         * @return bool ����������ɹ�����true��ʧ�ܷ���false��
         */
        bool createSocket();

        /**
         * @brief �󶨷������׽��ֵ�ָ���˿ڡ�
         *
         * @return bool ����������ɹ�����true��ʧ�ܷ���false��
         */
        bool bindSocket() const;

        /**
         * @brief ����ָ���˿��ϵ���������
         *
         * @return bool ����������ɹ�����true��ʧ�ܷ���false��
         */
        bool listenSocket() const;

        /**
         * @brief ���ܿͻ������Ӳ��������߳̽��д���
         *
         * @param handleFunction ����ָ�룬���ڴ�����յ���TCP��Ϣ��
         */
        void acceptConnections(std::string(*handleFunction)(const std::string&, const std::string&));

        /**
         * @brief ����ͻ������ӵĺ�����
         *
         * @param mtx ������������ͬ��������
         * @param log_opreations �Ƿ��¼������־�ı�־��
         * @param all_client_ip �ͻ���IP��ַ�б�
         * @param clientSocket �ͻ����׽��֡�
         * @param handleFunction ����ָ�룬���ڴ�����յ���TCP��Ϣ��
         */
        static void handleClient(std::shared_mutex& mtx, bool log_opreations, std::vector<std::string>& all_client_ip, SOCKET clientSocket, std::string(*handleFunction)(const std::string&, const std::string&));

        /**
         * @brief �رշ������׽��ֲ�������Դ��
         */
        void closeServer();
    };

}  // namespace ems
