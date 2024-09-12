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
     * @brief HTTP�������࣬���ڹ���HTTP�����API�󶨡�
     */
    class httpServer {
    private:

        std::string host;       ///<������������ַ��
        std::string port;       ///<�������˿ںš�
        std::string mount_dir;  ///<���ڹ��ؾ�̬�ļ���Ŀ¼��
        std::shared_mutex& mtx; ///<���������������߳�ͬ����
        bool log_operations;    ///<�Ƿ��¼������־��
        httplib::Server hvr;    ///<HTTP�������������ڴ�������

        /**
         * @brief ɾ���Ŀ������캯������ֹ���ơ�
         */
        httpServer(const httpServer&) = delete;

        /**
         * @brief ɾ���ĸ�ֵ����������ֹ��ֵ��
         */
        httpServer& operator=(const httpServer&) = delete;

        /**
         * @brief ��API�ӿڵ�ʵ�֡�
         */
        void bindApi();

        /**
         * @brief ����HTTP��������
         *
         * @return int ����״̬�롣
         */
        int run();

        /**
         * @brief ֹͣHTTP��������
         *
         * @return int ����״̬�롣
         */
        int stop();

        /**
         * @brief esysControl��Ϊ��Ԫ�࣬���Է���httpServer��˽�г�Ա��
         */
        friend class esysControl;

    public:

        /**
         * @brief ���캯������ʼ��HTTP����������
         *
         * @param mtx �������������ã������߳�ͬ����
         */
        httpServer(std::shared_mutex& mtx);
    };

} // namespace ems
