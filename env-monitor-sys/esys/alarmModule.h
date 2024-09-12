/**
 * @file alarmModule.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief Alert module, checks if the data exceeds the threshold, otherwise starts the alert program
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */


#pragma once
#include <unordered_map>
#include <set>
#include "esysControl.h"  // �����Զ���������

namespace ems {  // �����ռ� ems ��ʼ

	/**
	 * @class alarmModule
	 * @brief ���ڹ���ͼ�ر�����ģ�顣
	 *
	 * �����ṩ�˱�����ء���ֵ����ͱ�������ܣ�֧�ֶ��̻߳����µı���״̬����
	 */
	class alarmModule
	{
	private:
		/**
		 * @brief ���������ĳ���ʱ�䣨�룩��
		 */
		static int alarm_lock_duration_seconds;

		/**
		 * @brief �����������ڱ����������ݵķ��ʡ�
		 */
		static std::mutex mtx;

		/**
		 * @brief �������������ڶ��߳�ͬ����
		 */
		static std::condition_variable cv;

		/**
		 * @brief �������ü�ʱ���ı�־��key Ϊ IP ��ַ��value Ϊ�Ƿ����ü�ʱ����ԭ�Ӳ���ֵ��
		 */
		static std::unordered_map<std::string, std::atomic<bool>> reset_timer_active_ip;

		/**
		 * @brief ������ֵ��ӳ�䣬key Ϊ�������ͣ�value Ϊ��Ӧ����ֵ��
		 */
		static std::unordered_map<std::string, double> threshold;

		/**
		 * @brief �ͻ��˱���״̬��ӳ�䣬key Ϊ IP ��ַ��value Ϊ�Ƿ��ڱ���״̬��ԭ�Ӳ���ֵ��
		 */
		static std::unordered_map<std::string, std::atomic<bool>> alarm_active_ip;

		/**
		 * @brief �ͻ��˱�����Ϣ��ӳ�䣬key Ϊ IP ��ַ��value Ϊһ���ڲ�ӳ�䣬��ӳ��� key Ϊ�������ͣ�value Ϊ�����ľ���ֵ��
		 */
		static std::unordered_map<std::string, std::unordered_map<std::string, double>> alarm_active_message;

		/**
		 * @brief ������Ϣ���ַ���������
		 */
		std::string alarmMessage;

		/**
		 * @brief ˽�й��캯������ֹ���ʵ������
		 */
		alarmModule();

		/**
		 * @brief ˽������������
		 */
		~alarmModule() = default;

		/**
		 * @brief ɾ���������캯������ֹ������
		 */
		alarmModule(const alarmModule&) = delete;

		/**
		 * @brief ɾ����ֵ����������ֹ��ֵ��
		 */
		alarmModule& operator=(const alarmModule&) = delete;

		/**
		 * @brief ������ esysControl �����Ԫ������
		 */
		friend class esysControl;

		/**
		 * @brief ��ر���״̬��
		 *
		 * @param data ����������ݵ�ӳ�䡣
		 * @return std::string ������Ϣ���ַ���������
		 */
		std::string alarmMonitor(const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief ����ָ���ͻ��� IP �ı���״̬��
		 *
		 * @param clientIP �ͻ��˵� IP ��ַ��
		 */
		static void alarmActive(const std::string& clientIP);

		/**
		 * @brief �洢������Ϣ��ӳ�䣬key Ϊ�������ͣ�value Ϊ�����ľ���ֵ��
		 */
		std::map<std::string, std::string> message;

	public:
		/**
		 * @brief ��ȡ alarmModule ��ĵ���ʵ����
		 *
		 * @return alarmModule& ����ʵ�������á�
		 */
		static alarmModule& getInstance() {
			static alarmModule instance;
			return instance;
		}

		/**
		 * @brief ��ȡ��ǰ�ı�����Ϣ��
		 *
		 * @return std::map<std::string, std::string> ������Ϣ��ӳ�䣬key Ϊ�������ͣ�value Ϊ�����ľ���ֵ��
		 */
		std::map<std::string, std::string> getAlarmMessage();

		/**
		 * @brief ��ȡ��ǰ���õı�����ֵ��
		 *
		 * @return std::unordered_map<std::string, double> ������ֵ��ӳ�䣬key Ϊ�������ͣ�value Ϊ��ֵ��
		 */
		std::unordered_map<std::string, double> getThreshold();
	};
} // namespace ems ����
