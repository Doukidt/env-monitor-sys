/**
 * @file httpServer.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief The HTTP server module is used to process HTTP requests and provide API interfaces.
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */

#pragma once

#include "httplib.h"
#include "../esys/esysControl.h"

namespace ems {

    /**
     * @class httpServer
     * @brief HTTP服务器类，用于管理HTTP请求和API绑定。
     */
    class httpServer {
    private:

        std::string host;       ///<服务器主机地址。
        std::string port;       ///<服务器端口号。
        std::string mount_dir;  ///<用于挂载静态文件的目录。
        std::shared_mutex& mtx; ///<共享互斥锁，用于线程同步。
        bool log_operations;    ///<是否记录操作日志。
        httplib::Server hvr;    ///<HTTP服务器对象，用于处理请求。

        /**
         * @brief 删除的拷贝构造函数，防止复制。
         */
        httpServer(const httpServer&) = delete;

        /**
         * @brief 删除的赋值操作符，防止赋值。
         */
        httpServer& operator=(const httpServer&) = delete;

        /**
         * @brief 绑定API接口的实现。
         */
        void bindApi();

        /**
         * @brief 运行HTTP服务器。
         *
         * @return int 操作状态码。
         */
        int run();

        /**
         * @brief 停止HTTP服务器。
         *
         * @return int 操作状态码。
         */
        int stop();

        /**
         * @brief esysControl类为友元类，可以访问httpServer的私有成员。
         */
        friend class esysControl;

    public:

        /**
         * @brief 构造函数，初始化HTTP服务器对象。
         *
         * @param mtx 共享互斥锁的引用，用于线程同步。
         */
        httpServer(std::shared_mutex& mtx);
    };

} // namespace ems
