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

static constexpr int BUFFER_SIZE = 1024;  ///< 缓冲区大小，用于接收数据。

namespace ems {

    /**
     * @class tcpConnector
     * @brief 管理TCP服务器的连接和数据交换。
     */
    class tcpConnector {
    public:
        /**
         * @brief 构造函数，初始化TCP连接器。
         *
         * @param mtx 共享锁，用于同步服务器操作。
         */
        tcpConnector(std::shared_mutex& mtx);

        /**
         * @brief 析构函数，关闭服务器并清理资源。
         */
        ~tcpConnector();

        /**
         * @brief 启动TCP服务器。
         *
         * @param handleFunction 函数指针，用于处理接收到的TCP信息。
         * @return int 操作结果，成功返回0，失败返回1。
         */
        int startServer(std::string(*handleFunction)(const std::string&, const std::string&));

    private:
        unsigned short port;                            ///< 服务器监听端口。
        SOCKET serverSocket;                            ///< 服务器套接字。
        std::vector<std::thread> threads;               ///< 线程池，用于处理客户端连接。
        std::shared_mutex& mtx;                         ///< 共享锁，用于同步操作。
        bool log_operations;                            ///< 是否记录操作日志的标志。
        std::vector<std::string> all_client_ip;         ///< 所有连接的客户端IP地址列表。

        /**
         * @brief 初始化Winsock库。
         *
         * @return bool 操作结果，成功返回true，失败返回false。
         */
        bool initializeWinsock();

        /**
         * @brief 创建服务器套接字。
         *
         * @return bool 操作结果，成功返回true，失败返回false。
         */
        bool createSocket();

        /**
         * @brief 绑定服务器套接字到指定端口。
         *
         * @return bool 操作结果，成功返回true，失败返回false。
         */
        bool bindSocket() const;

        /**
         * @brief 监听指定端口上的连接请求。
         *
         * @return bool 操作结果，成功返回true，失败返回false。
         */
        bool listenSocket() const;

        /**
         * @brief 接受客户端连接并启动新线程进行处理。
         *
         * @param handleFunction 函数指针，用于处理接收到的TCP信息。
         */
        void acceptConnections(std::string(*handleFunction)(const std::string&, const std::string&));

        /**
         * @brief 处理客户端连接的函数。
         *
         * @param mtx 共享锁，用于同步操作。
         * @param log_opreations 是否记录操作日志的标志。
         * @param all_client_ip 客户端IP地址列表。
         * @param clientSocket 客户端套接字。
         * @param handleFunction 函数指针，用于处理接收到的TCP信息。
         */
        static void handleClient(std::shared_mutex& mtx, bool log_opreations, std::vector<std::string>& all_client_ip, SOCKET clientSocket, std::string(*handleFunction)(const std::string&, const std::string&));

        /**
         * @brief 关闭服务器套接字并清理资源。
         */
        void closeServer();
    };

}  // namespace ems
