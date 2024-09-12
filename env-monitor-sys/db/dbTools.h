/**
 * @file dbTools.h
 * @author Yilin Wang (yilin233@foxmail.com)
 * @brief A tool class used to handle database operations, supporting database 
 *  connection, insertion, read, query, and other operations.
 * @version 1.0
 * @date 2024-09-11
 *
 * @copyright Copyright (c) 2024 Yilin Wang
 *
 * MIT License
 */


#pragma once

// ����MySQL���ӺͲ��������ͷ�ļ�
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../esys/esysControl.h"  // �����Զ���������

namespace ems {  // namespace ems start

	/**
	 * @class dbTools
	 * @brief ���ڴ������ݿ�����Ĺ����֧࣬�����ݿ����ӡ����롢��ȡ����ѯ�Ȳ�����
	 */
	class dbTools {
	private:
		// MySQL���Ӷ���
		std::unique_ptr<sql::Connection> con;
		// ���ڻ����ṹ��Ϣ��ӳ��
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table_structure_cache;

		std::string url;				// ���ݿ�URL
		std::string user;				// ���ݿ��û���
		std::string password;			// ���ݿ�����
		std::string schema;				// ʹ�õ����ݿ�schema
		std::string build_file_location;// �������ݿ��ļ�λ��
		bool log_operations;			// �Ƿ��¼������־
		std::shared_mutex mtx;			// ��������������ͬ������

		/**
		 * @brief ˽�й��캯������ʼ�����ݿ����ӡ�
		 */
		dbTools();

		/**
		 * @brief ˽������������
		 */
		~dbTools() = default;

		/**
		 * @brief ɾ���������캯����
		 */
		dbTools(const dbTools&) = delete;

		/**
		 * @brief ɾ����ֵ��������
		 */
		dbTools& operator=(const dbTools&) = delete;

		/**
		 * @brief ִ��SQL��䡣
		 * @param sql Ҫִ�е�SQL����ַ�����
		 */
		void executeSQL(const std::string& sql);

		/**
		 * @brief ��ʼ�����ݿ����ӡ�
		 * @param url ���ݿ�URL��
		 * @param user ���ݿ��û�����
		 * @param password ���ݿ����롣
		 * @param schema ʹ�õ����ݿ�schema��
		 */
		void initConnection(const std::string& url, const std::string& user, const std::string& password, const std::string& schema);

		/**
		 * @brief ȥ���ַ������˵Ŀո�
		 * @param str Ҫȥ���ո���ַ�����
		 * @return std::string ȥ���ո����ַ�����
		 */
		std::string trim(const std::string& str);

	public:
		/**
		 * @brief ��ȡdbTools��ĵ���ʵ����
		 *
		 * @return dbTools& ����ʵ�������á�
		 */
		static dbTools& getInstance() {
			static dbTools instance;
			return instance;
		}

		/**
		 * @brief ��ȡָ����Ľṹ��Ϣ��
		 *
		 * @param table_name ������
		 * @return std::unordered_map<std::string, std::string> ��������͸������͵Ĺ�ϣ��
		 * @note �����ṹ�ѻ��棬��ֱ�Ӵӻ����л�ȡ��
		 */
		std::unordered_map<std::string, std::string> getTableStructure(const std::string& table_name);

		/**
		 * @brief ��ָ�����в���������ݡ�
		 *
		 * @param table_name ������
		 * @param data Ҫ����������б�vector<unordered_map<string ����, string ֵ>>��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbInsert(const std::string& table_name, const std::vector<std::unordered_map<std::string, std::string>>& data);

		/**
		 * @brief ��ָ�����в���һ�����ݡ�
		 *
		 * @param table_name ������
		 * @param data Ҫ��������ݣ�unordered_map<string ����, string ֵ>��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbInsert(const std::string& table_name, const std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief ��ָ�����ж�ȡ�������ݡ�
		 *
		 * @param table_name ������
		 * @param data ���ڴ洢��ȡ���ݵ����ã�vector<map<string ����, string ֵ>>��
		 * @param count_row Ҫ��ȡ��������
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbRead(const std::string& table_name, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		/**
		 * @brief ��ָ�����ж�ȡһ�����ݡ�
		 *
		 * @param table_name ������
		 * @param data ���ڴ洢��ȡ���ݵ����ã�unordered_map<string ����, string ֵ>��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbRead(const std::string& table_name, std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief ���ݿͻ���IP��ָ�����ж�ȡ�������ݡ�
		 *
		 * @param table_name ������
		 * @param client_ip �ͻ���IP��
		 * @param data ���ڴ洢��ȡ���ݵ����ã�vector<map<string ����, string ֵ>>��
		 * @param count_row Ҫ��ȡ��������
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::vector<std::unordered_map<std::string, std::string>>& data, unsigned int count_row);

		/**
		 * @brief ���ݿͻ���IP��ָ�����ж�ȡһ�����ݡ�
		 *
		 * @param table_name ������
		 * @param client_ip �ͻ���IP��
		 * @param data ���ڴ洢��ȡ���ݵ����ã�unordered_map<string ����, string ֵ>��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbReadByClientIP(const std::string& table_name, const std::string& client_ip, std::unordered_map<std::string, std::string>& data);

		/**
		 * @brief ��ָ�����в�ѯ���Ե�Ψһֵ��
		 *
		 * @param table_name ������
		 * @param attribute Ҫ��ѯ�����ԡ�
		 * @param data ���ڴ洢Ψһֵ��vector��
		 * @return int ����������ɹ����� EXIT_SUCCESS��ʧ�ܷ��� EXIT_FAILURE��
		 */
		int dbDistinctSelect(const std::string& table_name, const std::string& attribute, std::vector<std::string>& data);
	};

}  // namespace ems end
