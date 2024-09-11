#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>  // For inet_ntop
#include <shared_mutex>
#include "../esys/esysControl.h"
#pragma comment(lib, "ws2_32.lib")

static constexpr int BUFFER_SIZE = 1024;

namespace ems {

    class tcpConnector {
    public:
        /**
         * @brief ���캯������ʼ��TCP��������
         *
         * @param port �����������˿ڣ�Ĭ��Ϊ8080����
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
        unsigned short port;  // �����������˿�
        SOCKET serverSocket;  // �������׽���
        std::vector<std::thread> threads;  // �̳߳�
        std::shared_mutex& mtx; // ������
        bool log_operations;
        std::vector<std::string> all_client_ip;

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
         * @param clientSocket �ͻ����׽��֡�
         * @param handleFunction ����ָ�룬���ڴ�����յ���TCP��Ϣ��
         */
        static void handleClient(std::shared_mutex& mtx,bool log_opreations, std::vector<std::string>& all_client_ip, SOCKET clientSocket, std::string(*handleFunction)(const std::string&, const std::string&));

        /**
         * @brief �رշ������׽��ֲ�������Դ��
         */
        void closeServer();
    };

}  // namespace ems
